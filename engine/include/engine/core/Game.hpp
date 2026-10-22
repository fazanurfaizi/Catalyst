#pragma once

#include "engine/graphics/Camera.hpp"
#include "engine/scene/Scene.hpp"
#include <array>
#include <memory>
#include <string>

#include <GLFW/glfw3.h>

namespace engine {
class Renderer;
}

namespace engine {

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

  void SetClearColor(const glm::vec4 &color) { m_ClearColor = color; }

protected:
  virtual void OnInitialize() {}
  virtual void OnUpdate(double deltaTime) {}
  virtual void OnRender() {}
  virtual void OnShutdown() {}

  virtual void OnKey(int key, int scancode, int action, int mods) {}
  virtual void OnMouseButton(int button, int action, int mods) {}
  virtual void OnMouseMove(double x, double y) {}
  virtual void OnScroll(double dx, double dy) {}
  virtual void OnResize(int w, int h) {}

  virtual void OnImGui() {}

  bool IsKeyDown(int key) const;

  GLFWwindow *Window() const { return m_Window; }
  const WindowConfig &Config() const { return m_Config; }

  Camera &GetCamera() { return m_Camera; }
  const Camera &GetCamera() const { return m_Camera; }

  Scene &GetScene() { return m_Scene; }
  const Scene &GetScene() const { return m_Scene; }

  Renderer &GetRenderer() { return *m_Renderer; }
  const Renderer &GetRenderer() const { return *m_Renderer; }

private:
  WindowConfig m_Config{};
  GLFWwindow *m_Window{nullptr};
  Camera m_Camera;
  Scene m_Scene;
  std::unique_ptr<Renderer> m_Renderer;
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

} // namespace engine
