#pragma once

#include <glm/glm.hpp>
#include <string>

#include "engine/components/RenderLayer.hpp"

namespace engine {

class Texture;

struct Sprite {
  glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
  const Texture *texture = nullptr;
  glm::vec2 uvMin{0.0f, 0.0f};
  glm::vec2 uvMax{1.0f, 1.0f};
  RenderLayer layer{RenderLayer::Entities};
  float ySortKey = 0.0f;
  std::string regionName{};
};

} // namespace engine
