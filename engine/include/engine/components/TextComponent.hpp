#pragma once

#include "engine/asset/core/AssetHandle.hpp"
#include "engine/graphics/text/TextAlignment.hpp"
#include "engine/math/Color.hpp"
#include <glm/glm.hpp>
#include <string>

namespace engine::components {

struct TextComponent {
  std::string text;
  asset::AssetHandle font;
  math::Color color = math::kColorWhite;

  float scale = 1.0f;
  float maxWidth = 0.0f;

  graphics::TextAlignment alignment = graphics::TextAlignment::Left;
};

} // namespace engine::components
