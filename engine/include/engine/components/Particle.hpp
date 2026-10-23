#pragma once

#include <glm/glm.hpp>

namespace engine {

struct Particle {
  glm::vec2 velocity{0.0f};
  float life{1.0f};
  float maxLife{1.0f};
  float startScale{1.0f};
  float endScale{1.0f};
  glm::vec4 startColor{1.0f};
  glm::vec4 endColor{1.0f};
};

} // namespace engine
