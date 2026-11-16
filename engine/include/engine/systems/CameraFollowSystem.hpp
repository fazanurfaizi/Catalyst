#pragma once

#include "engine/components/Camera2D.hpp"
#include "engine/components/CameraFollow.hpp"
#include "engine/components/Transform.hpp"
#include "engine/ecs/Registry.hpp"

namespace engine::systems {

inline void CameraFollowSystem(ecs::Registry &reg, float dt) {
  for (auto [e, cam, follow] :
       reg.View<components::Camera2D, components::CameraFollow>()) {
    if (!cam.active)
      continue;
    if (!follow.target.IsValid())
      continue;

    const auto *t = reg.TryGet<components::Transform>(follow.target);
    if (!t)
      continue;

    const glm::vec2 target = t->position + follow.offset;
    if (follow.speed > 0.0f) {
      cam.position =
          glm::mix(cam.position, target, glm::min(1.0f, follow.speed * dt));
    } else {
      cam.position = target;
    }
  }
}

} // namespace engine::systems
