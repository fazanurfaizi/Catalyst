#pragma once

#include <glm/glm.hpp>

namespace game {

// Move toward target
struct Velocity {
  glm::vec2 targetPosition{0.0f, 0.0f};
  float speed{2.0f};

  [[nodiscard]] bool ArrivedAtTarget(glm::vec2 pos,
                                     float epsilon = 0.05f) const noexcept {
    const glm::vec2 distance = targetPosition - pos;
    return glm::dot(distance, distance) <= epsilon * epsilon;
  }
};

// Projectile/physics/AI linear
struct LinearVelocity {
  glm::vec2 value{0.0f, 0.0f};
};

} // namespace game
