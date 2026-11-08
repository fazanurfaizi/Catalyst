#include "engine/core/Game.hpp"

#include "engine/asset/AssetManager.hpp"
#include "engine/asset/manager/AssetManagerEditor.hpp"
#include "engine/asset/manager/AssetManagerRuntime.hpp"
#include "engine/core/Clock.hpp"
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

namespace engine::core {

namespace {
void GlfwErrorCallback(int code, const char *desc) {
  std::fprintf(stderr, "GLFW error %d: %s\n", code, desc);
}
} // namespace

Game::Game(const platform::WindowConfig &config, const DebugConfig &debug)
    : m_config(config), m_debug(debug) {
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
}

Game::~Game() {
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
    Logger::Error("Game::Run callced without a renderer");
    return false;
  }

  if (!audio::AudioEngine::Get().Init()) {
    Logger::Error("Audio failed to initialiez; continuing without audio");
  }

  asset::AssetManager::RegisterDefaultImporters(m_importerRegistry);
  m_assets = std::make_unique<asset::AssetManagerEditor>(
      m_importerRegistry,
      "" // Dev mode: playing state passes absolute paths
  );

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

} // namespace engine::core
