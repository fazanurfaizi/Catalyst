#pragma once

#include <glm/glm.hpp>

namespace engine {

struct Velocity {
  glm::vec2 targetPosition{0.0f, 0.0f};
  float speed{2.0f};
};

} // namespace engine
