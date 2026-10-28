#pragma once

#include "engine/input/InputSystem.hpp"
#include "engine/platform/Display.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include <glm/glm.hpp>

struct GLFWwindow;

namespace engine::platform {

struct WindowConfig {
  std::string title = "Untitled"; // game sets the real title
  int width = 1280;
  int height = 720;
  std::int8_t monitorIndex = -1;
  bool vsync = true;
  bool resizable = true;
  bool fullscreen = false;
};

inline constexpr std::size_t kMaxKeys = 512;

struct WindowEvents {
  std::function<void(input::Key key, int scancode, input::KeyAction action,
                     int mods)>
      onKey;
  std::function<void(input::MouseButton button, input::KeyAction action,
                     int mods)>
      onMouseButton;
  std::function<void(double x, double y)> onCursorMove;
  std::function<void(double x, double y)> onScroll;
  std::function<void(int width, int height)> onResize;
  std::function<void()> onClose;
};

class Window {
public:
  static std::unique_ptr<Window> Create(const WindowConfig &config);

  explicit Window(const WindowConfig &config);
  ~Window();
  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

  // Lifecycle
  bool ShouldClose() const;
  void RequestClose();
  void PollEvents();
  void SwapBuffers();
  void ClearWindow();

  // Info
  Display &GetDisplay() { return m_display; }
  const Display &GetDisplay() const { return m_display; }

  // Display mode
  bool IsFullscreen() const { return m_fullscreen; }
  void SetFullscreen(bool fullscreen);
  void ToggleFullscreen() { SetFullscreen(!m_fullscreen); }
  void SetWindowedSize(int w, int h);
  void SetVsync(bool enabled);

  void SetTitle(std::string_view title);

  // Input
  bool IsKeyDown(int key) const;
  input::InputSystem &Input() { return m_input; }
  const input::InputSystem &Input() const { return m_input; }

  GLFWwindow *GlfwHandle() const { return m_window; }

  WindowEvents events;

protected:
  static void ThunkKey(GLFWwindow *window, int key, int scancode, int action,
                       int mods);
  static void ThunkMouseButton(GLFWwindow *window, int button, int action,
                               int mods);
  static void ThunkCursorPos(GLFWwindow *window, double x, double y);
  static void ThunkScroll(GLFWwindow *window, double x, double y);
  static void ThunkFramebufferResize(GLFWwindow *window, int width, int height);
  static void ThunkClose(GLFWwindow *w);

  static Window *Self(GLFWwindow *w);

private:
  WindowConfig m_Config{};
  Display m_display;
  GLFWwindow *m_window{nullptr};

  bool m_fullscreen = false;
  int m_savedX = 0, m_savedY = 0;
  int m_savedW = 1280, m_savedH = 720;

  input::InputSystem m_input;
};

} // namespace engine::platform
