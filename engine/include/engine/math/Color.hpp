
#pragma once

#include "Math.hpp"

namespace engine::math {

/// Floating-point RGBA. 1.0 == full intensity.
struct Color {
  float r = 1.0f;
  float g = 1.0f;
  float b = 1.0f;
  float a = 1.0f;

  constexpr Color() = default;
  constexpr Color(float r_, float g_, float b_, float a_ = 1.0f)
      : r(r_), g(g_), b(b_), a(a_) {}

  bool operator==(const Color &) const = default;

  [[nodiscard]] constexpr glm::vec4 toVec4() const { return {r, g, b, a}; }

  [[nodiscard]] constexpr Color withAlpha(float alpha) const {
    return {r, g, b, alpha};
  }

  [[nodiscard]] static constexpr Color fromVec4(glm::vec4 v) noexcept {
    return {v.x, v.y, v.z, v.w};
  }
};

inline constexpr Color kColorWhite{1.0f, 1.0f, 1.0f, 1.0f};
inline constexpr Color kColorBlack{0.0f, 0.0f, 0.0f, 1.0f};
inline constexpr Color kColorTransparent{0.0f, 0.0f, 0.0f, 0.0f};

inline Color lerp(const Color &a, const Color &b, float t) {
  return {lerpf(a.r, b.r, t), lerpf(a.g, b.g, t), lerpf(a.b, b.b, t),
          lerpf(a.a, b.a, t)};
}

} // namespace engine::math
