#pragma once

#include "engine/graphics/assets/FontAsset.hpp"
#include "engine/graphics/gl/Shader.hpp"
#include "engine/graphics/text/TextLayout.hpp"

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace engine::graphics {

class TextRenderer {
public:
  TextRenderer();
  ~TextRenderer();

  void Draw(const FontAsset &asset, const std::string &text,
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
