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
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
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
