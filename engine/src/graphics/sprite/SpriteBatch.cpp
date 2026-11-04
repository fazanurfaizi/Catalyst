#include "engine/graphics/sprite/SpriteBatch.hpp"
#include "engine/assets/AssetPath.hpp"
#include "engine/graphics/sprite/SpriteDrawList.hpp"
#include "engine/graphics/texture/Texture.hpp"
#include "engine/utils/Logger.hpp"
#include "glad/gl.h"

#include <cstddef>

namespace engine::graphics {

SpriteBatch::SpriteBatch() {
  if (!m_shader.LoadFromFile(engine::AssetPath("shaders/sprite"))) {
    Logger::Error("[SpriteBatch] shader compilation failed");
    return;
  }

  // clang-format off
  constexpr float kCorners[] = {
    -0.5f, -0.5f,
    0.5f, -0.5f,
    0.5f, 0.5f,
    -0.5f, 0.5f
  };
  // clang-format on

  constexpr std::uint32_t kIndices[] = {0, 1, 2, 2, 3, 0};

  glGenVertexArrays(1, &m_vao);
  glBindVertexArray(m_vao);

  glGenBuffers(1, &m_staticVbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_staticVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kCorners), kCorners, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);

  glGenBuffers(1, &m_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices,
               GL_STATIC_DRAW);

  // Streaming instance buffer. glBufferData() below orphans/renames the
  // store on every upload, so draws still in flight keep reading the old
  // store instead of the data we overwrite this frame.
  glGenBuffers(1, &m_instanceVbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_instanceVbo);

  const GLsizei kStride = sizeof(SpriteInstance);

  // Position
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
      1, 2, GL_FLOAT, GL_FALSE, kStride,
      reinterpret_cast<const void *>(offsetof(SpriteInstance, position)));
  glVertexAttribDivisor(1, 1);

  // Scale
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(
      2, 2, GL_FLOAT, GL_FALSE, kStride,
      reinterpret_cast<const void *>(offsetof(SpriteInstance, scale)));
  glVertexAttribDivisor(2, 1);

  // rotation
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(
      3, 1, GL_FLOAT, GL_FALSE, kStride,
      reinterpret_cast<const void *>(offsetof(SpriteInstance, rotation)));
  glVertexAttribDivisor(3, 1);

  // color
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(
      4, 4, GL_FLOAT, GL_FALSE, kStride,
      reinterpret_cast<const void *>(offsetof(SpriteInstance, color)));
  glVertexAttribDivisor(4, 1);

  // uvRect
  glEnableVertexAttribArray(5);
  glVertexAttribPointer(
      5, 4, GL_FLOAT, GL_FALSE, kStride,
      reinterpret_cast<const void *>(offsetof(SpriteInstance, uvRect)));
  glVertexAttribDivisor(5, 1);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  m_defaultTexture.CreateSolid(0xFFFFFFFF);
}

SpriteBatch::~SpriteBatch() { Release(); }

void SpriteBatch::Release() {
  if (m_ibo)
    glDeleteBuffers(1, &m_ibo);
  if (m_staticVbo)
    glDeleteBuffers(1, &m_staticVbo);
  if (m_instanceVbo)
    glDeleteBuffers(1, &m_instanceVbo);
  if (m_vao)
    glDeleteVertexArrays(1, &m_vao);
  m_ibo = m_staticVbo = m_instanceVbo = m_vao = 0;
}

void SpriteBatch::Begin(const glm::mat4 &viewProjection) {
  m_viewProjection = viewProjection;
  m_lastDrawCalls = 0;
  m_lastInstanceCount = 0;
}

void SpriteBatch::DrawInstances(const SpriteInstance *data, std::size_t count,
                                const Texture *texture) {
  if (!data || count == 0)
    return;

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_instanceVbo);
  glBufferData(GL_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(count * sizeof(SpriteInstance)), data,
               GL_STREAM_DRAW);

  m_shader.Bind();
  m_shader.SetMat4("uViewProjection", m_viewProjection);
  // glUniformMatrix4fv(m_uViewport, 1, GL_FALSE, &m_viewProjection[0][0]);
  // glUniform1i(m_uTexture, 0);
  m_shader.SetInt("uTexture", 0);
  const Texture *tex = texture ? texture : &m_defaultTexture;
  tex->Bind(0);

  glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr,
                          static_cast<GLsizei>(count));
  ++m_lastDrawCalls;
  m_lastInstanceCount += static_cast<std::uint32_t>(count);
}

void SpriteBatch::End() {}

} // namespace engine::graphics
