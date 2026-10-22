#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace engine {

struct AnimationFrame {
  glm::vec2 uvMin{0.0f, 0.0f};
  glm::vec2 uvMax{1.0f, 1.0f};
  float durationSec = 0.1f;
};

struct AnimationClip {
  std::string name;
  std::vector<AnimationFrame> frames;
  bool loop = true;

  bool IsValid() const { return !frames.empty(); }
  int FrameCount() const { return static_cast<int>(frames.size()); }
  float TotalDuration() const {
    float t = 0.0f;
    for (const auto &f : frames) {
      t += f.durationSec;
    }
    return t;
  }
};

} // namespace engine
