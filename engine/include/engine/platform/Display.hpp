#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct GLFWwindow;

namespace engine::platform {

class Display {
public:
  void Initialize(GLFWwindow *window) {
    m_window = window;
    Refresh();
  }
  void Refresh();
  void Shutdown() { m_window = nullptr; }

  void OnWindowResize(int w, int h);
  void OnFramebufferResize(int w, int h);

  glm::ivec2 WindowSize() const { return m_windowSize; }
  glm::ivec2 FramebufferSize() const { return m_framebufferSize; }
  int Width() const { return m_framebufferSize.x; }
  int Height() const { return m_framebufferSize.y; }

  float AspectRatio() const {
    if (m_framebufferSize.y <= 0) {
      return 1.0f;
    }
    return static_cast<float>(m_framebufferSize.x) /
           static_cast<float>(m_framebufferSize.y);
  }

  glm::vec2 PixelScale() const {
    if (m_windowSize.x <= 0 || m_windowSize.y <= 0) {
      return {1.0f, 1.0f};
    }
    return {static_cast<float>(m_framebufferSize.x) /
                static_cast<float>(m_windowSize.x),
            static_cast<float>(m_framebufferSize.y) /
                static_cast<float>(m_windowSize.y)};
  }

  glm::vec2 CursorToFramebuffer(glm::vec2 cursor) const {
    return cursor * PixelScale();
  }

  const glm::mat4 &UiProjection() const;

private:
  GLFWwindow *m_window = nullptr;
  glm::ivec2 m_windowSize{1280, 720};
  glm::ivec2 m_framebufferSize{1280, 720};
  mutable glm::mat4 m_uiProjection{1.0f};
  mutable bool m_uiDirty = true;
};

} // namespace engine::platform
