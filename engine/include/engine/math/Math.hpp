#pragma once

#include "glm/exponential.hpp"
#include "glm/geometric.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace engine::math {

inline float clampf(float v, float lo, float hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}

inline float lerpf(float a, float b, float t) { return a + (b - a) * t; }

inline glm::vec3 toVec3(const glm::vec4 v) { return {v.r, v.g, v.b}; }

inline glm::vec3 safeNormalize(const glm::vec3 &v, glm::vec3 fallback) {
  const float len2 = glm::dot(v, v);
  return len2 > 1e-8f ? v / glm::sqrt(len2) : fallback;
}

} // namespace engine::math
