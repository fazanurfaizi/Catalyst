#pragma once

#include "engine/math/Color.hpp"
#include <glm/glm.hpp>

namespace game {

struct Particle {
  glm::vec2 velocity{0.0f};
  float life{1.0f};
  float maxLife{1.0f};
  float startScale{1.0f};
  float endScale{1.0f};
  engine::Color startColor = engine::kWhite;
  engine::Color endColor = engine::kWhite;
};

} // namespace game
