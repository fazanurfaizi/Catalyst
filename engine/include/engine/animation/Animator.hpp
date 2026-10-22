#pragma once

#include "engine/animation/AnimationClip.hpp"

namespace engine {

struct Animator {
  const AnimationClip *clip = nullptr;
  int currentFrame = 0;
  float elapsedSec = 0.0f;
  bool playing = true;
  bool finished = false;

  bool IsActive() const {
    return clip != nullptr && clip->IsValid() && playing;
  }

  void Play(const AnimationClip *c) {
    clip = c;
    currentFrame = 0;
    elapsedSec = 0.0f;
    playing = (c != nullptr && c->IsValid());
    finished = false;
  }

  void Stop() {
    clip = nullptr;
    playing = false;
    finished = false;
    elapsedSec = 0.0f;
    currentFrame = 0;
  }

  void Advance(float deltaTime) {
    if (!IsActive())
      return;

    elapsedSec += deltaTime;

    while (elapsedSec >= clip->frames[currentFrame].durationSec) {
      elapsedSec -= clip->frames[currentFrame].durationSec;
      ++currentFrame;

      if (currentFrame >= clip->FrameCount()) {
        if (clip->loop) {
          currentFrame = 0;
        } else {
          currentFrame = clip->FrameCount() - 1;
          playing = false;
          finished = true;
          elapsedSec = 0.0f;
          return;
        }
      }
    }
  }
};

} // namespace engine
