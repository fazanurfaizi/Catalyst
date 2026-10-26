#pragma once

#include <glm/glm.hpp>

#include "RenderLayer.hpp"
#include "engine/math/Color.hpp"

namespace engine::graphics {
class Texture;
}

namespace engine::components {

struct Sprite {
  math::Color color = engine::math::kColorWhite;
  const graphics::Texture *texture = nullptr;
  glm::vec2 uvMin{0.0f, 0.0f};
  glm::vec2 uvMax{1.0f, 1.0f};
  RenderLayer layer{RenderLayer::Entities};
  float ySortKey = 0.0f;
  std::uint16_t regionId = 0;
};

} // namespace engine::components
