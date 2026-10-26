#pragma once

#include <glm/glm.hpp>

namespace engine::components {

// Pure kinematics (no solver constraints). Sufficient for platform games.
// Bullets and particles move.
struct RigidBody {
  glm::vec2 velocity{0.0f, 0.0f};
  glm::vec2 acceleration{0.0f, 0.0f};
  float mass = 1.0f;
  float drag = 0.0f;
  float restitution = 0.0f;
  float gravityScale = 0.0f;
  bool isStatic = false;
};

} // namespace game
