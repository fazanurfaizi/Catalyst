#pragma once

#include "engine/ecs/Registry.hpp"
#include "game/components/Transform.hpp"
#include "game/components/Velocity.hpp"
#include <glm/glm.hpp>

namespace game::systems {

// Move toward target
inline void MoveTowardTargetSytem(engine::ecs::Registry &reg, float dt) {
  for (auto [e, t, v] : reg.View<Transform, Velocity>()) {
    const glm::vec2 delta = v.targetPosition - t.position;
    const float dist2 = glm::dot(delta, delta);
    if (dist2 < 0.05f * 0.05f) {
      t.position = v.targetPosition;
      continue;
    }
    const float dist = glm::sqrt(dist2);
    const float step = glm::min(v.speed * dt, dist);
    t.position += (delta / dist) * step;
    if (delta.x > 0)
      t.scale.x = std::abs(t.scale.x);
    else if (delta.x < 0)
      t.scale.x = -std::abs(t.scale.x);
  }
}

// Projectile linear movement
inline void LinearIntegrateSystem(engine::ecs::Registry &reg, float dt) {
  for (auto [e, t, v] : reg.View<Transform, LinearVelocity>()) {
    t.position += v.value * dt;
  }
}

} // namespace game::systems
