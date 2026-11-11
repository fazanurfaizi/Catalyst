#pragma once

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace engine::components {

struct AnimationFrame {
  std::string regionName;
  float durationSec = 0.1f;
};

inline void to_json(nlohmann::json &j, const AnimationFrame &f) {
  j = nlohmann::json{
      {"regionName", f.regionName},
      {"durationSec", f.durationSec},
  };
}

inline void from_json(const nlohmann::json &j, AnimationFrame &f) {
  j.at("regionName").get_to(f.regionName);
  j.at("durationSec").get_to(f.durationSec);
}

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

} // namespace engine::components
