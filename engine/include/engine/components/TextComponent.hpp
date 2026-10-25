#pragma once

#include <glm/glm.hpp>
#include <string>

namespace engine {

class Font;

struct TextComponent {
  std::string text;
  const Font *font = nullptr;
  glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
  float scale = 1.0f;
};

} // namespace engine
