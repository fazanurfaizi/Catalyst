#pragma once

#include "engine/ecs/Registry.hpp"
#include "game/components/Camera2D.hpp"
#include "game/components/Transform.hpp"

namespace game::systems {

inline void CameraFollowSystem(engine::ecs::Registry &reg, float dt) {
  for (auto [e, cam] : reg.View<Camera2D>()) {
    if (!cam.active)
      continue;
    if (!cam.followTarget.IsValid())
      continue;

    const auto *t = reg.TryGet<game::Transform>(cam.followTarget);
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

} // namespace game::systems
