#pragma once

#include "engine/ecs/Entity.hpp"

#include <glm/glm.hpp>

namespace engine::components {

struct CameraFollow {
  engine::ecs::Entity target{};
  float speed = 8.0f;
  glm::vec2 offset{0.0f, 0.0f}; // world-space offset applied to the target
};

} // namespace engine::components
