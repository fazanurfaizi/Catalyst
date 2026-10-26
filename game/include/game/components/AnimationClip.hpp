#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace game {

struct AnimationFrame {
  glm::vec2 uvMin{0.0f, 0.0f};
  glm::vec2 uvMax{1.0f, 1.0f};
  float durationSec = 0.1f;
};

struct AnimationClip {
  std::string name;
  std::vector<AnimationFrame> frames;
  bool loop = true;

  [[nodiscard]] bool IsValid() const noexcept { return !frames.empty(); }
  [[nodiscard]] int FrameCount() const noexcept {
    return static_cast<int>(frames.size());
  }

  [[nodiscard]] float TotalDuration() const noexcept {
    float t = 0.0f;
    for (const auto &f : frames) {
      t += f.durationSec;
    }
    return t;
  }
};

} // namespace game
