#include "engine/platform/Display.hpp"
#include "glm/ext/matrix_clip_space.hpp"

#include <GLFW/glfw3.h>

namespace engine::platform {

void Display::Refresh() {
  if (!m_window)
    return;

  int windowW, windowH;
  int framebufferW, framebufferH;
  glfwGetWindowSize(m_window, &windowW, &windowH);
  glfwGetFramebufferSize(m_window, &framebufferW, &framebufferH);
  OnWindowResize(windowW, windowH);
  OnFramebufferResize(framebufferW, framebufferH);
}

void Display::OnWindowResize(int w, int h) {
  m_windowSize = {w > 0 ? w : 1, h > 0 ? h : 1};
  m_uiDirty = true;
}

void Display::OnFramebufferResize(int w, int h) {
  m_framebufferSize = {w > 0 ? w : 1, h > 0 ? h : 1};
  m_uiDirty = true;
}

const glm::mat4 &Display::UiProjection() const {
  if (m_uiDirty) {
    m_uiProjection =
        glm::ortho(0.0f, static_cast<float>(m_framebufferSize.x),
                   static_cast<float>(m_framebufferSize.y), 0.0f, -1.0f, 1.0f);
    m_uiDirty = false;
  }
  return m_uiProjection;
}

} // namespace engine::platform
