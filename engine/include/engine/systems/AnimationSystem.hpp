#pragma once

#include "engine/animation/AnimationLibrary.hpp"
#include "engine/components/Animator.hpp"
#include "engine/components/Sprite.hpp"
#include "engine/ecs/Registry.hpp"

namespace engine::systems {

inline void AnimationSystem(ecs::Registry &reg,
                            const animation::AnimationLibrary &library,
                            float dt) {
  for (auto [e, anim, sprite] :
       reg.View<components::Animator, components::Sprite>()) {

    if (!anim.playing || !anim.HasClip())
      continue;

    const auto *clip = library.Get(anim.clipName);
    if (!clip || !clip->IsValid())
      continue;

    if (anim.currentFrameDuration <= 0.0f &&
        anim.currentFrame < clip->FrameCount()) {
      anim.currentFrameDuration = clip->frames[anim.currentFrame].durationSec;
    }

    anim.elapsedSec += dt;
    if (anim.elapsedSec >= anim.currentFrameDuration) {
      const int n = clip->FrameCount();

      while (anim.elapsedSec >= anim.currentFrameDuration) {
        anim.elapsedSec -= anim.currentFrameDuration;
        ++anim.currentFrame;

        if (anim.currentFrame >= n) {
          if (clip->loop) {
            anim.currentFrame = 0;
          } else {
            anim.currentFrame = n - 1;
            anim.playing = false;
            anim.finished = true;
            anim.elapsedSec = 0.0f;
            anim.currentFrameDuration =
                clip->frames[anim.currentFrame].durationSec;
            break;
          }
        }
        anim.currentFrameDuration = clip->frames[anim.currentFrame].durationSec;
      }
    }

    const auto &frame = clip->frames[anim.currentFrame];
    sprite.atlas = anim.atlas;
    sprite.regionName = frame.regionName;
  }
}

} // namespace engine::systems
