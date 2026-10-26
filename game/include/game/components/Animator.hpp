#pragma once

#include "AnimationClip.hpp"

namespace game {

struct Animator {
  const AnimationClip *clip = nullptr;
  int currentFrame = 0;
  float elapsedSec = 0.0f;
  float currentFrameDuration = 0.0f;
  bool playing = true;
  bool finished = false;

  [[nodiscard]] bool IsActive() const noexcept {
    return clip != nullptr && clip->IsValid() && playing;
  }

  void Play(const AnimationClip *c) {
    clip = c;
    currentFrame = 0;
    elapsedSec = 0.0f;
    playing = (c != nullptr && c->IsValid());
    finished = false;
    currentFrameDuration = playing ? c->frames[0].durationSec : 0.0f;
  }

  void Stop() {
    clip = nullptr;
    playing = false;
    finished = false;
    elapsedSec = 0.0f;
    currentFrame = 0;
    currentFrameDuration = 0.0f;
  }

  void Advance(float deltaTime) {
    if (!IsActive())
      return;

    elapsedSec += deltaTime;

    if (elapsedSec < currentFrameDuration)
      return;

    const int n = clip->FrameCount();

    while (elapsedSec >= currentFrameDuration) {
      elapsedSec -= currentFrameDuration;
      ++currentFrame;

      if (currentFrame >= n) {
        if (clip->loop) {
          currentFrame = 0;
        } else {
          currentFrame = n - 1;
          playing = false;
          finished = true;
          elapsedSec = 0.0f;
          currentFrameDuration = clip->frames[currentFrame].durationSec;
          return;
        }
      }
      currentFrameDuration = clip->frames[currentFrame].durationSec;
    }
  }
};

} // namespace game
