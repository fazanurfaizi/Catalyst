#pragma once

#include "engine/math/Color.hpp"
#include <glm/glm.hpp>

namespace engine::components {

struct Particle {
  glm::vec2 velocity{0.0f};
  float life{1.0f};
  float maxLife{1.0f};
  float startScale{1.0f};
  float endScale{1.0f};
  math::Color startColor = math::kColorWhite;
  math::Color endColor = math::kColorWhite;
};

} // namespace engine::components
