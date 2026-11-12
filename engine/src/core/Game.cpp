#include "engine/core/Game.hpp"

#include "engine/asset/manager/AssetManagerRuntime.hpp"
#include "engine/audio/AudioEngine.hpp"
#include "engine/components/AnimationClip.hpp"
#include "engine/components/Animator.hpp"
#include "engine/components/AudioSource.hpp"
#include "engine/components/Camera2D.hpp"
#include "engine/components/Light.hpp"
#include "engine/components/Name.hpp"
#include "engine/components/Particle.hpp"
#include "engine/components/ParticleEmitter.hpp"
#include "engine/components/Sprite.hpp"
#include "engine/components/TextComponent.hpp"
#include "engine/components/Transform.hpp"
#include "engine/components/Velocity.hpp"
#include "engine/core/Clock.hpp"
#include "engine/core/GameConfigSerializer.hpp"
#include "engine/ecs/ComponentSerializer.hpp"
#include "engine/graphics/Renderer.hpp"
#include "engine/utils/Logger.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// @NOTE: glfwGetProcAddress for glad loader
#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <cstdio>
#include <memory>
#include <utility>

namespace engine::core {

namespace {
void GlfwErrorCallback(int code, const char *desc) {
  std::fprintf(stderr, "GLFW error %d: %s\n", code, desc);
}

void InitializeComponentSerialization() {
  auto &reg = ecs::ComponentRegistry::Instance();

  reg.Register<components::AnimationFrame>();
  reg.Register<components::AnimationClip>();
  reg.Register<components::Animator>();
  reg.Register<components::AudioSource>();
  reg.Register<components::Camera2D>();
  reg.Register<components::PointLight>();
  reg.Register<components::DirectionalLight>();
  reg.Register<components::SpotLight>();
  reg.Register<components::Name>();
  reg.Register<components::Particle>();
  reg.Register<components::ParticleEmitter>();
  reg.Register<components::Sprite>();
  reg.Register<components::TextComponent>();
  reg.Register<components::Transform>();
  reg.Register<components::Velocity>();
  reg.Register<components::LinearVelocity>();
}
} // namespace

Game::Game(const platform::WindowConfig &config, const DebugConfig &debug,
           std::filesystem::path gameConfigPath)
    : m_config(config), m_debug(debug),
      m_gameConfigPath(std::move(gameConfigPath)) {

  m_window = platform::Window::Create(m_config);
  if (!m_window) {
    Logger::Error("glfwInit failed");
    return;
  }

  if (!gladLoadGL(glfwGetProcAddress)) {
    Logger::Error("gladLoadGL failed");
    m_window.reset();
    return;
  }

  m_renderer = std::make_unique<graphics::Renderer>();
  const auto fb = m_window->GetDisplay().FramebufferSize();
  m_renderer->Init(fb.x, fb.y);

  // Wire window events -> state manager
  m_window->events.onKey = [this](input::Key key, int sc,
                                  input::KeyAction action, int mods) {
    m_stateManager.OnKey(*this, key, sc, action, mods);
  };
  m_window->events.onMouseButton = [this](input::MouseButton button,
                                          input::KeyAction action, int mods) {
    m_stateManager.OnMouseButton(*this, button, action, mods);
  };
  m_window->events.onCursorMove = [this](double x, double y) {
    m_stateManager.OnMouseMove(*this, x, y);
  };
  m_window->events.onScroll = [this](double x, double y) {
    m_stateManager.OnScroll(*this, x, y);
  };
  m_window->events.onResize = [this](int w, int h) {
    m_stateManager.OnResize(*this, w, h);
  };

  if (m_debug.enableImGui) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_window->GlfwHandle(), /*install_cb=*/true);
    ImGui_ImplOpenGL3_Init("#version 330");
    m_imguiInited = true;
  }

  Logger::Info("GL: %s",
               reinterpret_cast<const char *>(glGetString(GL_VERSION)));

  InitializeComponentSerialization();
}

Game::~Game() {
#ifdef CATALYST_EDITOR
  if (m_watcher) {
    m_watcher->Stop();
  }
#endif // CATALYST_EDITOR

  m_stateManager.Clear(*this);

  audio::AudioEngine::Get().Shutdown();
  m_assets.reset();

  if (m_imguiInited) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }
}

double Game::ElapsedTime() const { return m_clock.ElapsedTime(); }

void Game::SetFullscreen(bool fullscreen) {
  if (!m_window)
    return;
  m_window->SetFullscreen(fullscreen);
  m_config.fullscreen = m_window->IsFullscreen();
  m_config.width = m_window->GetDisplay().WindowSize().x;
  m_config.height = m_window->GetDisplay().WindowSize().y;
}

void Game::ToggleFullscreen() { SetFullscreen(!IsFullscreen()); }

bool Game::IsFullscreen() const { return m_window && m_window->IsFullscreen(); }

bool Game::Run() {
  if (!m_window) {
    Logger::Error("Game::Run called without a valid window");
    return false;
  }
  if (!m_renderer) {
    Logger::Error("Game::Run called without a renderer");
    return false;
  }

  if (!audio::AudioEngine::Get().Init()) {
    Logger::Error("Audio failed to initialiez; continuing without audio");
  }

  asset::AssetModule::RegisterDefaultImporters(m_importerRegistry);

  auto configOpt = GameConfigSerializer::Load(m_gameConfigPath);
  if (!configOpt) {
    Logger::Error("Game::Run: could not load '%s'; running with no assets",
                  m_gameConfigPath.c_str());
    m_gameConfig = GameConfig{};
    m_gameConfig.root = std::filesystem::current_path();
  } else {
    m_gameConfig = std::move(*configOpt);
    Logger::Info("Loaded game config: '%s' (%zu assets, root '%s')",
                 m_gameConfig.projectName.c_str(), m_gameConfig.assets.size(),
                 m_gameConfig.root.c_str());
  }

  m_assets = std::make_unique<asset::AssetManagerRuntime>(m_importerRegistry,
                                                          m_gameConfig.root);

  for (auto &record : m_gameConfig.assets) {
    m_assets->AddRecord(record);
  }

  for (const auto &record : m_gameConfig.assets) {
    m_assets->RefreshSettingsHash(record.handle);
  }

#ifdef CATALYST_EDITOR
  m_watcher =
      std::make_unique<asset::AssetWatcher>(std::chrono::milliseconds(500));

  for (const auto &record : m_gameConfig.assets) {
    if (record.state != asset::AssetRecordState::Valid)
      continue;

    const std::filesystem::path abs = m_gameConfig.root / record.path;
    const std::filesystem::path parent = abs.parent_path();

    m_watcher->WatchDirectory(record.handle, parent);

    Logger::Info("[Game] watching '%s' for handle '%s'", parent.c_str(),
                 record.name.empty() ? record.path.c_str()
                                     : record.name.c_str());
  }
  m_watcher->Start();
#endif

  OnStart();

  m_window->PollEvents();
  m_window->GetDisplay().Refresh();
  {
    const auto fb = m_window->GetDisplay().FramebufferSize();
    if (fb.x > 0 && fb.y > 0) {
      m_renderer->Resize(fb.x, fb.y);
    }
  }

  m_running = true;
  m_clock.Reset();

  while (m_running && !CloseRequested()) {
    audio::AudioEngine::Get().Update();

#ifdef CATALYST_EDITOR
    if (m_watcher) {
      for (const auto &ev : m_watcher->Poll()) {
        Logger::Info("[hot-reload] %s", ev.path.c_str());
        m_assets->Reimport(ev.handle);
      }
    }
#endif // CATALYST_EDITOR

    m_window->Input().BeginFrame();
    m_window->PollEvents();

    const auto fb = m_window->GetDisplay().FramebufferSize();
    if (fb.x > 0 && fb.y > 0 &&
        (fb.x != m_renderer->Width() || fb.y != m_renderer->Height())) {
      m_renderer->Resize(fb.x, fb.y);
    }

    m_clock.Tick();

    while (m_clock.ShouldConsumeStep()) {
      m_stateManager.FixedUpdate(*this, m_clock.FixedDeltaTime());
      m_clock.ConsumeStep();
    }

    m_stateManager.Update(*this, m_clock.DeltaTime());

    m_stateManager.LateUpdate(*this, m_clock.DeltaTime());

    m_window->ClearWindow();
    m_stateManager.Render(*this);

    if (m_debug.showFps) {
      m_fpsAccum += m_clock.DeltaTime();
      m_fpsFrames += 1;
      if (m_fpsAccum >= 0.5) {
        const double fps = m_fpsFrames / m_fpsAccum;
        const double ms = 1000.0 / (fps > 1e-3 ? fps : 1.0);
        char buf[160];
        std::snprintf(buf, sizeof(buf), "%s  |  %.0f FPS  |  %.2f ms",
                      m_config.title.c_str(), fps, ms);
        m_window->SetTitle(buf);
        m_fpsAccum = 0.0;
        m_fpsFrames = 0;
      }
    }

    if (m_imguiInited) {
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      m_stateManager.OnImGui(*this);

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    m_window->SwapBuffers();
  }

  OnShutdown();

  return true;
}

void Game::SyncConfigFromAssets() {
  if (!m_assets)
    return;

  for (auto &record : m_gameConfig.assets) {
    const auto live = m_assets->RecordCopy(record.handle);
    if (!live)
      continue;

    record.settingsHash = live->settingsHash;
    record.state = live->state;
    record.dependencies = live->dependencies;
  }
}

} // namespace engine::core
