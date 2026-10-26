#pragma once

#include "engine/math/Color.hpp"
#include <glm/glm.hpp>
#include <string>

namespace engine {
class Font;
}

namespace engine::components {

struct TextComponent {
  std::string text;
  const Font *font = nullptr;
  math::Color color = math::kColorWhite;
};

} // namespace engine::components
