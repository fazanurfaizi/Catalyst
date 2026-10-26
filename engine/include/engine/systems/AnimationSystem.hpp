#pragma once

#include "engine/components/Animator.hpp"
#include "engine/components/Sprite.hpp"
#include "engine/ecs/Registry.hpp"

namespace engine::systems {

inline void AnimationSystem(ecs::Registry &reg, float dt) {
  for (auto [e, anim, sprite] :
       reg.View<components::Animator, components::Sprite>()) {
    if (!anim.IsActive())
      continue;
    anim.Advance(dt);
    const auto &frame = anim.clip->frames[anim.currentFrame];
    sprite.uvMin = frame.uvMin;
    sprite.uvMax = frame.uvMax;
  }
}

} // namespace engine::systems
