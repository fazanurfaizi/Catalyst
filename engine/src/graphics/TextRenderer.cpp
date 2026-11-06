#include "engine/graphics/TextRenderer.hpp"

#include "engine/asset/AssetPath.hpp"
#include "engine/graphics/text/TextLayout.hpp"
#include "engine/utils/Logger.hpp"
#include "glad/gl.h"

#include <cstddef>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

namespace engine::graphics {

TextRenderer::TextRenderer() {
  if (!m_shader.LoadFromFile(AssetPath("shaders/text"))) {
    Logger::Error("[TextRenderer] Failed to compile text shader");
    return;
  }

  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

  // Initial allocation
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 6, nullptr, GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(
      0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      reinterpret_cast<const void *>(offsetof(Vertex, position)));

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
      1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      reinterpret_cast<const void *>(offsetof(Vertex, texCoords)));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

TextRenderer::~TextRenderer() {
  glDeleteVertexArrays(1, &m_vao);
  glDeleteBuffers(1, &m_vbo);
}

void TextRenderer::Draw(const Font &font, const std::string &text,
                        const glm::vec2 &position, float scale,
                        const glm::vec4 &color, const glm::mat4 &projection,
                        float maxWidth, TextAlignment alignment) {
  if (text.empty())
    return;

  TextLayout::BuildTextBatch(font, text, maxWidth, scale, alignment, m_vertices,
                             position);

  if (m_vertices.empty())
    return;

  const GLboolean depthWasOn = glIsEnabled(GL_DEPTH_TEST);
  glDisable(GL_DEPTH_TEST);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  m_shader.Bind();
  m_shader.SetMat4("uProjection", projection);
  m_shader.SetVec4("uTextColor", color);
  m_shader.SetInt("uText", 0);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, font.GetTextureId());
  glBindVertexArray(m_vao);

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(m_vertices.size() * sizeof(Vertex)),
               m_vertices.data(), GL_DYNAMIC_DRAW);
  glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertices.size()));

  if (depthWasOn)
    glEnable(GL_DEPTH_TEST);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  m_shader.Unbind();
}

} // namespace engine::graphics
