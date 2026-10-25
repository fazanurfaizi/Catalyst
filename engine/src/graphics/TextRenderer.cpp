#include "engine/graphics/TextRenderer.hpp"

#include "engine/core/Logger.hpp"
#include "glad/gl.h"

#include <glm/gtc/type_ptr.hpp>
#include <vector>

namespace engine {

const char *vShaderSrc = R"(
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;
uniform mat4 projection;
void main() {
  gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
  TexCoords = vertex.zw;
}
)";

const char *fShaderSrc = R"(
  #version 330 core
  in vec2 TexCoords;
  out vec4 color;
  uniform sampler2D text;
  uniform vec4 textColor;
  void main() {
    float alpha = texture(text, TexCoords).r;
    color = vec4(textColor.rgb, textColor.a * alpha);
  }
)";

TextRenderer::TextRenderer() {
  if (!m_shader.Compile(vShaderSrc, fShaderSrc)) {
    Logger::Error("[TextRenderer] Failed to compile text shader");
    return;
  }

  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

  // Initial allocation
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr,
               GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

TextRenderer::~TextRenderer() {
  glDeleteVertexArrays(1, &m_vao);
  glDeleteBuffers(1, &m_vbo);
}

void TextRenderer::Draw(const Font &font, const std::string &text,
                        const glm::vec2 &position, float scale,
                        const glm::vec4 &color, const glm::mat4 &projection) {
  if (text.empty())
    return;

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  m_shader.Bind();
  m_shader.SetMat4("projection", projection);
  m_shader.SetVec4("textColor", color);
  m_shader.SetInt("text", 0);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, font.GetTextureId());
  glBindVertexArray(m_vao);

  float x = position.x;
  float y = position.y;

  // Pre-allocate vertex data for the entire string to bacth into a single draw
  // call
  std::vector<float> vertices;
  vertices.reserve(text.length() * 24);

  for (char c : text) {
    try {
      const Character &ch = font.GetCharacter(c);
      float xpos = x + ch.bearing.x * scale;
      float ypos = y + (ch.bearing.y * scale);

      float w = ch.size.x * scale;
      float h = ch.size.y * scale;

      if (w > 0.0f && h > 0.0f) {
        // clang-format off
        float charVertices[6][4] = {
          // Top-Left (ypos is top)
          {xpos,     ypos,     ch.uvMin.x, ch.uvMax.y},
          // Bottom-Left (ypos + h is bottom)
          {xpos,     ypos + h, ch.uvMin.x, ch.uvMin.y},
          // Bottom-Right
          {xpos + w, ypos + h, ch.uvMax.x, ch.uvMin.y},

          // Top-Left
          {xpos,     ypos,     ch.uvMin.x, ch.uvMax.y},
          // Bottom-Right
          {xpos + w, ypos + h, ch.uvMax.x, ch.uvMin.y},
          // Top-Right
          {xpos + w, ypos,     ch.uvMax.x, ch.uvMax.y},
        };
        // clang-format on

        for (int i = 0; i < 6; ++i) {
          for (int j = 0; j < 4; ++j) {
            vertices.push_back(charVertices[i][j]);
          }
        }
      }

      x += ch.advance * scale;
    } catch (...) {
      Logger::Warn("[TextRenderer] character %s is not found in font atlas map",
                   c);
      continue;
    }
  }

  if (!vertices.empty()) {
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                 vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size() / 4));
  }

  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  m_shader.Unbind();
}

} // namespace engine
