#pragma once

#include <glm/glm.hpp>

namespace engine::components {

struct BoxCollider {
  glm::vec2 halfExtents{0.5f, 0.5f};
  glm::vec2 offset{0.0f, 0.0f};
  bool isTrigger = false;

  [[nodiscard]] glm::vec2 Center(glm::vec2 pos) const noexcept {
    return pos + offset;
  }
};

} // namespace engine::components
