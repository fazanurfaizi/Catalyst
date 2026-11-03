#pragma once

#include "engine/math/Color.hpp"
#include <glm/glm.hpp>

namespace engine::graphics {

struct alignas(16) PointLightGpu {
  glm::vec4 position;
  glm::vec4 ambient;
  glm::vec4 diffuse;
  glm::vec4 specular;
  glm::vec4 attenuation; // x=constant, y=linear, z=quadratic, w unused
};
static_assert(sizeof(PointLightGpu) == 80, "PointLightGpu must be 80 bytes");

struct alignas(16) DirectionalLightGpu {
  glm::vec4 direction;
  math::Color color;
};
static_assert(sizeof(DirectionalLightGpu) == 32,
              "DirectionalLightGpu must be 32 bytes");

struct alignas(16) SpotLightGpu {
  glm::vec4 position;
  glm::vec4 direction;
  math::Color color;
  glm::vec4 cone;        // x = innerCutoff (cos), y = outerCutoff (cos)
  glm::vec4 attenuation; // x=constant, y=linear, z=quadratic
};

static_assert(sizeof(SpotLightGpu) == 80, "SpotLightGpu must be 80 bytes");

struct LightBuffer {
  static constexpr int kMaxPoint = 64;
  static constexpr int kMaxDirectional = 4;
  static constexpr int kMaxSpot = 16;

  int pointCount = 0;
  int directionalCount = 0;
  int spotCount = 0;

  DirectionalLightGpu directional[kMaxDirectional]{};
  PointLightGpu points[kMaxPoint];
  SpotLightGpu spots[kMaxSpot];
};

} // namespace engine::graphics
