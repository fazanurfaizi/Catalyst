#pragma once

#include "engine/math/Color.hpp"
#include <glm/glm.hpp>
#include <string>

namespace game {

class Font;

struct TextComponent {
  std::string text;
  const Font *font = nullptr;
  engine::Color color = engine::kWhite;
};

} // namespace game
