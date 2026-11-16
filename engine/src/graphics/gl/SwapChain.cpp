#include "engine/graphics/gl/SwapChain.hpp"

#include <GLFW/glfw3.h>
#include <glad/gl.h>

namespace engine::graphics {

SwapChain::~SwapChain() { Shutdown(); }

void SwapChain::Initialize(GLFWwindow *window) { m_window = window; }

void SwapChain::Shutdown() {
  m_window = nullptr;
  m_captureRequested = false;
}

void SwapChain::RequestCapture() { m_captureRequested = true; }

bool SwapChain::ReadBackCapture(std::vector<std::uint8_t> &out, int &w,
                                int &h) {
  w = h = 0;
  if (!m_captureRequested || !m_window)
    return false;
  m_captureRequested = false;

  int fbW = 0, fbH = 0;
  glfwGetFramebufferSize(m_window, &fbW, &fbH);
  if (fbW <= 0 || fbH <= 0)
    return false;
  w = fbW;
  h = fbH;

  GLuint tex = 0, fbo = 0;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fbW, fbH, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, nullptr);

  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, tex, 0);
  glBlitFramebuffer(0, 0, fbW, fbH, 0, 0, fbW, fbH, GL_COLOR_BUFFER_BIT,
                    GL_NEAREST);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
  out.resize(static_cast<std::size_t>(fbW) * static_cast<std::size_t>(fbH) *
             4u);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, fbW, fbH, GL_RGBA, GL_UNSIGNED_BYTE, out.data());

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &fbo);
  glDeleteTextures(1, &tex);
  return true;
}

void SwapChain::Present() {
  if (m_window)
    glfwSwapBuffers(m_window);
}

} // namespace engine::graphics
