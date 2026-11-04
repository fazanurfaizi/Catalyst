#pragma once

#include <array>
#include <glad/gl.h>

namespace engine::graphics {

class GLStateCache {
public:
  void UseProgram(GLuint program) {
    if (program != m_program) {
      glUseProgram(program);
      m_program = program;
    }
  }

  void BindVAO(GLuint vao) {
    if (vao != m_vao) {
      glBindVertexArray(vao);
      m_vao = vao;
    }
  }

  void BindTexture2D(int unit, GLuint tex) {
    if (unit < 0 || unit >= static_cast<int>(m_tex.size()))
      return;

    if (m_tex[unit] == tex)
      return;

    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, tex);
    m_tex[unit] = tex;
    m_activeUnit = unit;
  }

  void BindFBO(GLuint fbo, int x, int y, int w, int h) {
    if (fbo != m_fbo) {
      glBindFramebuffer(GL_FRAMEBUFFER, fbo);
      m_fbo = fbo;
    }

    if (x != m_viewport[0] || y != m_viewport[1] || w != m_viewport[2] ||
        h != m_viewport[3]) {
      glViewport(x, y, w, h);
      m_viewport = {x, y, w, h};
    }
  }

  void InvalidateAll() {
    m_program = 0xFFFFFFFFu;
    m_vao = 0xFFFFFFFFu;
    m_fbo = 0xFFFFFFFFu;
    m_tex.fill(0xFFFFFFFFu);
    m_activeUnit = -1;
    m_viewport = {-1, -1, -1, -1};
  }

  GLuint CurrentProgram() const { return m_program; }
  GLuint CurrentVAO() const { return m_vao; }
  GLuint CurrentFBO() const { return m_fbo; }

private:
  GLuint m_program = 0xFFFFFFFFu;
  GLuint m_vao = 0xFFFFFFFFu;
  GLuint m_fbo = 0xFFFFFFFFu;
  int m_activeUnit = -1;
  std::array<GLuint, 16> m_tex{};
  std::array<int, 4> m_viewport{-1, -1, -1, -1};
};

} // namespace engine::graphics
