#pragma once

#include "engine/asset/AssetModule.hpp"
#include "engine/asset/importer/ImporterRegistry.hpp"
#include "engine/asset/manager/IAssetManager.hpp"
#include "engine/audio/AudioEngine.hpp"
#include "engine/core/Clock.hpp"
#include "engine/core/GameConfig.hpp"
#include "engine/core/GameStateManager.hpp"
#include "engine/platform/Window.hpp"

#ifdef CATALYST_EDITOR
#include "engine/asset/watcher/AssetWatcher.hpp"
#endif // CATALYST_EDITOR

#include <filesystem>
#include <memory>

#include <glm/glm.hpp>

struct GLFWwindow;

namespace engine::graphics {
class Renderer;
}

namespace engine::core {

struct DebugConfig {
  bool enableImGui = true;
  bool showFps = true;
};

class Game {
public:
  explicit Game(const platform::WindowConfig &config,
                const DebugConfig &debug = {},
                std::filesystem::path gameConfigPath = "game.json");
  virtual ~Game();

  Game(const Game &) = delete;
  Game &operator=(const Game &) = delete;

  bool Run();

  void SetState(std::unique_ptr<GameState> state) {
    m_stateManager.ChangeState(*this, std::move(state));
  }

  platform::Window &GetWindow() { return *m_window; }
  const platform::Window &GetWindow() const { return *m_window; }

  input::InputSystem &Input() { return m_window->Input(); }
  const input::InputSystem &Input() const { return m_window->Input(); }

  bool IsKeyDown(input::Key key) const {
    return m_window && m_window->Input().IsKeyDown(key);
  }
  bool IsMouseDown(input::MouseButton b) const {
    return m_window && m_window->Input().IsMouseDown(b);
  }

  void RequestClose() {
    if (m_window)
      m_window->RequestClose();
  }
  bool CloseRequested() const {
    return !m_window || m_window->ShouldClose() ||
           m_window->Input().CloseRequested();
  }

  double ElapsedTime() const;

  const platform::WindowConfig &Config() const { return m_config; }
  const DebugConfig &Debug() const { return m_debug; }

  platform::Display &Display() { return m_window->GetDisplay(); }
  const platform::Display &Display() const { return m_window->GetDisplay(); }

  void SetFullscreen(bool fullscreen);
  void ToggleFullscreen();
  bool IsFullscreen() const;

  void SetWindowedSize(int w, int h) { m_window->SetWindowedSize(w, h); }
  void SetVsync(bool on) { m_window->SetVsync(on); }

  graphics::Renderer &GetRenderer() { return *m_renderer; }
  const graphics::Renderer &GetRenderer() const { return *m_renderer; }

  asset::IAssetManager *Assets() { return m_assets.get(); }
  const asset::IAssetManager *Assets() const { return m_assets.get(); }

  audio::AudioEngine &GetAudio() { return audio::AudioEngine::Get(); }
  const audio::AudioEngine &GetAudio() const {
    return audio::AudioEngine::Get();
  }

  void SyncConfigFromAssets();
  const GameConfig &GetGameConfig() const { return m_gameConfig; }

#ifdef CATALYST_EDITOR
  asset::AssetWatcher *GetWatcher() { return m_watcher.get(); }
#endif // CATALYST_EDITOR

protected:
  virtual void OnStart() {}
  virtual void OnShutdown() {}

private:
  platform::WindowConfig m_config{};
  DebugConfig m_debug{};
  std::filesystem::path m_gameConfigPath;

  std::unique_ptr<platform::Window> m_window;

  GameStateManager m_stateManager;

  asset::ImporterRegistry m_importerRegistry;
  std::unique_ptr<asset::IAssetManager> m_assets;

  GameConfig m_gameConfig{};

  std::unique_ptr<graphics::Renderer> m_renderer;

#ifdef CATALYST_EDITOR
  std::unique_ptr<asset::AssetWatcher> m_watcher;
#endif // CATALYST_EDITOR

  Clock m_clock;

  bool m_running{false};
  bool m_imguiInited{false};
  double m_fpsAccum = 0.0;
  int m_fpsFrames = 0;

  static void ThunkKey(GLFWwindow *window, int key, int scancode, int action,
                       int mods);
  static void ThunkMouseButton(GLFWwindow *window, int button, int action,
                               int mods);
  static void ThunkCursorPos(GLFWwindow *window, double x, double y);
  static void ThunkScroll(GLFWwindow *window, double x, double y);
  static void ThunkFramebufferSize(GLFWwindow *window, int width, int height);
};

} // namespace engine::core
