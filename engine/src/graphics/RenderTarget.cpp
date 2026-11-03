#include "engine/graphics/RenderTarget.hpp"
#include "engine/utils/Logger.hpp"

#include <glad/gl.h>

namespace engine::graphics {

RenderTarget::~RenderTarget() { Release(); }

void RenderTarget::Release() {
  if (m_color) {
    glDeleteTextures(1, &m_color);
    m_color = 0;
  }
  if (m_depth) {
    glDeleteRenderbuffers(1, &m_depth);
    m_depth = 0;
  }
  if (m_fbo) {
    glDeleteFramebuffers(1, &m_fbo);
    m_fbo = 0;
  }
}

void RenderTarget::Create(int w, int h) {
  if (w <= 0 || h <= 0) {
    Logger::Error("[RenderTarget] invalid size %dx%d", w, h);
    return;
  }

  if (m_fbo != 0 && m_width == w && m_height == h)
    return;

  Release();
  m_width = w;
  m_height = h;

  glGenFramebuffers(1, &m_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

  // Color - RGBA16F so accumulative light never clamps at 1.0.
  glGenTextures(1, &m_color);
  glBindTexture(GL_TEXTURE_2D, m_color);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT,
               nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         m_color, 0);

  // Depth
  glGenRenderbuffers(1, &m_depth);
  glBindRenderbuffer(GL_RENDERBUFFER, m_depth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, m_depth);

  const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    Logger::Error("[RenderTarget] incomplete FBO: 0x%04X", status);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderTarget::Resize(int w, int h) { Create(w, h); }

void RenderTarget::Bind(int viewportX, int viewportY, int viewportW,
                        int viewportH) const {
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glViewport(viewportX, viewportY, viewportW, viewportH);
}

void RenderTarget::Unbind(int viewportX, int viewportY, int viewportW,
                          int viewportH) const {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(viewportX, viewportY, viewportW, viewportH);
}

void RenderTarget::BindColorTexture(std::uint32_t unit) const {
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, m_color);
}

} // namespace engine::graphics
