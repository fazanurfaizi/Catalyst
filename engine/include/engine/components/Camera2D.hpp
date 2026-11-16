#pragma once

#include "engine/math/Camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace engine::components {

struct Camera2D {
  glm::vec2 position{0.0f};
  float zoom = 1.0f;
  float rotation = 0.0f; // radians
  float orthoSize = 10.0f;

  bool active = true;

  void SetPosition(glm::vec2 p) noexcept { position = p; }
  void Move(glm::vec2 d) noexcept { position += d; }
  void SetZoom(float z) noexcept { zoom = z > 1e-4f ? z : 1e-4f; }
  void ZoomBy(float f) noexcept { SetZoom(zoom * f); }
  void SetRotation(float r) noexcept { rotation = r; }
  void RotateBy(float r) noexcept { rotation += r; }

  [[nodiscard]] glm::mat4 ViewMatrix() const noexcept {
    glm::mat4 m(1.0f);
    if (rotation != 0.0f) {
      m = glm::rotate(m, -rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    }
    m = glm::translate(m, glm::vec3(-position, 0.0f));
    return m;
  }

  [[nodiscard]] glm::mat4 ViewProjectionMatrix(float aspect) const noexcept {
    const float h = orthoSize / zoom;
    const float w = h * aspect;
    return glm::ortho(-w, w, -h, h, -1.0f, 1.0f) * ViewMatrix();
  }

  /// Build the renderer-facing value description of this camera.
  [[nodiscard]] math::Camera View(float aspect) const noexcept {
    return math::Camera::FromViewProjection(ViewProjectionMatrix(aspect),
                                            aspect);
  }
};

} // namespace engine::components
