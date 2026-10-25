#pragma once

#include "engine/graphics/Font.hpp"
#include "engine/graphics/Shader.hpp"

#include <glm/glm.hpp>
#include <string>

namespace engine {

class TextRenderer {
public:
  TextRenderer();
  ~TextRenderer();

  void Draw(const Font &font, const std::string &text,
            const glm::vec2 &position, float scale, const glm::vec4 &color,
            const glm::mat4 &projection);

private:
  GLuint m_vao = 0;
  GLuint m_vbo = 0;
  Shader m_shader;
};

} // namespace engine
