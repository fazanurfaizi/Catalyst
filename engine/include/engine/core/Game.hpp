#pragma once

#include "engine/core/GameStateManager.hpp"
#include <array>
#include <memory>
#include <string>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace engine::graphics {
class Renderer;
}

namespace engine::core {

struct WindowConfig {
  int width = 1280;
  int height = 720;
  std::string title = "Kingdom Rush Frontiers";
  bool vsync = true;
  bool resizable = true;
  bool enableImGui = true;
  bool showFps = true;
};

class Game {
public:
  explicit Game(const WindowConfig &config);
  virtual ~Game();

  Game(const Game &) = delete;
  Game &operator=(const Game &) = delete;

  bool Run();

  void SetState(std::unique_ptr<GameState> state) {
    m_stateManager.ChangeState(*this, std::move(state));
  }

  void SetClearColor(const glm::vec4 &color) { m_ClearColor = color; }

  bool IsKeyDown(int key) const;

  GLFWwindow *Window() const { return m_Window; }
  const WindowConfig &Config() const { return m_Config; }

  graphics::Renderer &GetRenderer() { return *m_Renderer; }
  const graphics::Renderer &GetRenderer() const { return *m_Renderer; }

protected:
  virtual void OnStart() {}
  // virtual void OnUpdate(double deltaTime) {}
  // virtual void OnRender() {}
  virtual void OnShutdown() {}

private:
  WindowConfig m_Config{};
  GLFWwindow *m_Window{nullptr};

  GameStateManager m_stateManager;
  std::unique_ptr<graphics::Renderer> m_Renderer;

  glm::vec4 m_ClearColor{0.15f, 0.20f, 0.30f, 1.0f};

  bool m_Running{false};
  bool m_ImguiInited{false};
  double m_FpsAccum = 0.0;
  int m_FpsFrames = 0;

  static void ThunkKey(GLFWwindow *window, int key, int scancode, int action,
                       int mods);
  static void ThunkMouseButton(GLFWwindow *window, int button, int action,
                               int mods);
  static void ThunkCursorPos(GLFWwindow *window, double x, double y);
  static void ThunkScroll(GLFWwindow *window, double x, double y);
  static void ThunkFramebufferSize(GLFWwindow *window, int width, int height);

  static Game *FromWindow(GLFWwindow *w) {
    return static_cast<Game *>(glfwGetWindowUserPointer(w));
  }

  std::array<bool, GLFW_KEY_LAST + 1> m_Keys{};
};

} // namespace engine::core
