#pragma once

#include "glm/ext/matrix_transform.hpp"
#include <glm/glm.hpp>

namespace engine::components {

struct Transform {
  glm::vec2 position{0.0f, 0.0f};
  glm::vec2 scale{1.0f, 1.0f};
  float rotation = 0.0f; // radians

  [[nodiscard]] glm::mat4 Matrix() const noexcept {
    glm::mat4 m(1.0f);
    m = glm::translate(m, glm::vec3(position, 0.0f));
    m = glm::rotate(m, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    m = glm::scale(m, glm::vec3(scale, 1.0f));
    return m;
  }

  [[nodiscard]] glm::vec2 Forward() const noexcept {
    return {glm::cos(rotation), glm::sin(rotation)};
  }
};

} // namespace game
