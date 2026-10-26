#pragma once

#include "engine/components/Camera2D.hpp"
#include "engine/components/Transform.hpp"
#include "engine/ecs/Registry.hpp"

namespace engine::systems {

inline void CameraFollowSystem(ecs::Registry &reg, float dt) {
  for (auto [e, cam] : reg.View<components::Camera2D>()) {
    if (!cam.active)
      continue;
    if (!cam.followTarget.IsValid())
      continue;

    const auto *t = reg.TryGet<components::Transform>(cam.followTarget);
    if (!t)
      continue;

    const glm::vec2 target = t->position + cam.followOffset;
    if (cam.followSpeed > 0.0f) {
      cam.position =
          glm::mix(cam.position, target, glm::min(1.0f, cam.followSpeed * dt));
    } else {
      cam.position = target;
    }
  }
}

} // namespace engine::systems
