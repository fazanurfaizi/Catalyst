#pragma once

#include "engine/asset/core/AssetHandle.hpp"
#include <string>
#include <string_view>

namespace engine::components {

struct Animator {
  asset::AssetHandle atlas{};
  std::string clipName;

  int currentFrame = 0;
  float elapsedSec = 0.0f;
  float currentFrameDuration = 0.0f;
  bool playing = true;
  bool finished = false;

  [[nodiscard]] bool HasClip() const noexcept { return !clipName.empty(); }
};

inline void PlayAnimation(Animator &a, asset::AssetHandle atlas,
                          std::string_view clipName) {
  a.atlas = atlas;
  a.clipName = std::string(clipName);
  a.currentFrame = 0;
  a.elapsedSec = 0.0f;
  a.playing = true;
  a.finished = false;
  a.currentFrameDuration = 0.0f;
}

inline void StopAnimation(Animator &a) {
  a.clipName.clear();
  a.playing = false;
  a.finished = false;
  a.elapsedSec = 0.0f;
  a.currentFrame = 0;
  a.currentFrameDuration = 0.0f;
}

} // namespace engine::components
