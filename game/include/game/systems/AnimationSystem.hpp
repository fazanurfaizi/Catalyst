#pragma once

#include "engine/ecs/Registry.hpp"
#include "game/components/Animator.hpp"
#include "game/components/Sprite.hpp"

namespace game::systems {

inline void AnimationSystem(engine::ecs::Registry &reg, float dt) {
  for (auto [e, anim, sprite] : reg.View<Animator, Sprite>()) {
    if (!anim.IsActive())
      continue;
    anim.Advance(dt);
    const auto &frame = anim.clip->frames[anim.currentFrame];
    sprite.uvMin = frame.uvMin;
    sprite.uvMax = frame.uvMax;
  }
}

} // namespace game::systems
