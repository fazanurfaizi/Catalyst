#pragma once

#include "engine/math/Color.hpp"
#include <glm/glm.hpp>

namespace engine::components {

struct DirectionalLight {
  math::Color color = math::kColorWhite;
  glm::vec3 direction{0.0f, -1.0f, -1.0f};
  float intensity{1.0f};
};

struct PointLight {
  math::Color color = math::kColorWhite;
  glm::vec3 ambient{0.15f, 0.08f, 0.02f};
  glm::vec3 diffuse{1.00f, 0.65f, 0.25f};
  glm::vec3 specular{0.00f, 0.00f, 0.00f};
  float constant{1.0f};
  float linear{0.9f};
  float quadratic{0.032};
};

struct SpotLight {
  math::Color color = math::kColorWhite;
  glm::vec3 direction{0.0f, -1.0f, -1.0f};
  float intensity{1.0f};
  float viewAngle{0.91f};
  float fadeAngle{0.8f};
  float radius{15.0f};
};

} // namespace engine::components
