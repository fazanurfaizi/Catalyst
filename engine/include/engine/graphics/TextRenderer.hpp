#pragma once

#include "engine/graphics/Font.hpp"
#include "engine/graphics/Shader.hpp"
#include "engine/graphics/TextLayout.hpp"

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace engine::graphics {

class TextRenderer {
public:
  TextRenderer();
  ~TextRenderer();

  void Draw(const Font &font, const std::string &text,
            const glm::vec2 &position, float scale, const glm::vec4 &color,
            const glm::mat4 &projection, float maxWidth = 0.0f,
            TextAlignment alignment = TextAlignment::Left);

private:
  GLuint m_vao = 0;
  GLuint m_vbo = 0;
  Shader m_shader;

  std::vector<Vertex> m_vertices;
};

} // namespace engine::graphics
