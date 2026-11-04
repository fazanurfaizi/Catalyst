#include "engine/graphics/pipeline/FullscreenQuad.hpp"

#include <glad/gl.h>

namespace engine::graphics {

FullscreenQuad::FullscreenQuad() {
  // clang-format off
  constexpr float kPositions[] = {
    -1.0f, -1.0f,
    1.0f, -1.0f,
    1.0f,  1.0f,
    -1.0f,  1.0f,
  };
  // clang-format on
  constexpr std::uint32_t kIndicies[] = {0, 1, 2, 2, 3, 0};

  glGenVertexArrays(1, &m_vao);
  glBindVertexArray(m_vao);

  glGenBuffers(1, &m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kPositions), kPositions, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                        reinterpret_cast<const void *>(0));

  glGenBuffers(1, &m_ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndicies), kIndicies,
               GL_STATIC_DRAW);

  glBindVertexArray(0);
}

FullscreenQuad::~FullscreenQuad() {
  if (m_ebo)
    glDeleteBuffers(1, &m_ebo);
  if (m_vbo)
    glDeleteBuffers(1, &m_vbo);
  if (m_vao)
    glDeleteVertexArrays(1, &m_vao);
}

void FullscreenQuad::Draw() const {
  glBindVertexArray(m_vao);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);
}

} // namespace engine::graphics
