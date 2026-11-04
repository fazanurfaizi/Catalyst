#pragma once

#include "engine/ecs/Entity.hpp"
#include "engine/math/Rect.hpp"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace engine::components {

struct Camera2D {
  glm::vec2 position{0.0f};
  float zoom = 1.0f;
  float rotation = 0.0f; // radians
  float orthoSize = 10.0f;

  // Game behavior
  bool active = true;
  engine::ecs::Entity followTarget{};
  float followSpeed = 8.0f;
  glm::vec2 followOffset{0.0f, 0.0f};

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

  [[nodiscard]] glm::vec2 WorldToNdc(const glm::vec2 &world,
                                     float aspect) const noexcept {
    glm::vec4 clip =
        ViewProjectionMatrix(aspect) * glm::vec4(world, 0.0f, 1.0f);
    if (clip.w == 0.0f)
      return glm::vec2(0.0f);
    return glm::vec2(clip) / clip.w;
  }

  [[nodiscard]] glm::vec2 NdcToWorld(const glm::vec2 &ndc,
                                     float aspect) const noexcept {
    glm::vec4 world =
        glm::inverse(ViewProjectionMatrix(aspect)) * glm::vec4(ndc, 0.0f, 1.0f);
    if (world.w == 0.0f)
      return glm::vec2(0.0f);
    return glm::vec2(world) / world.w;
  }

  [[nodiscard]] glm::vec2
  WorldToScreen(const glm::vec2 &world,
                const glm::ivec2 &fbSize) const noexcept {
    const float w = static_cast<float>(fbSize.x > 0 ? fbSize.x : 1);
    const float h = static_cast<float>(fbSize.y > 0 ? fbSize.y : 1);
    const glm::vec2 ndc = WorldToNdc(world, w / h);
    return {(ndc.x * 0.5f + 0.5f) * w, (0.5f - ndc.y * 0.5f) * h};
  }

  [[nodiscard]] glm::vec2
  ScreenToWorld(const glm::vec2 &screen,
                const glm::ivec2 &fbSize) const noexcept {
    const float w = float(fbSize.x > 0 ? fbSize.x : 1);
    const float h = float(fbSize.y > 0 ? fbSize.y : 1);
    const float ndcX = (screen.x / w) * 2.0f - 1.0f;
    const float ndcY = 1.0f - (screen.y / h) * 2.0f;
    return NdcToWorld({ndcX, ndcY}, w / h);
  }

  [[nodiscard]] math::Rect VisibleWorldBounds(float aspect) const noexcept {
    const glm::mat4 invVp = glm::inverse(ViewProjectionMatrix(aspect));

    auto corner = [&](float nx, float ny) {
      glm::vec4 w = invVp * glm::vec4(nx, ny, 0.0f, 1.0f);
      return glm::vec2(w) / w.w;
    };

    const auto tl = corner(-1.0f, 1.0f);
    const auto tr = corner(1.0f, 1.0f);
    const auto bl = corner(-1.0f, -1.0f);
    const auto br = corner(1.0f, -1.0f);

    float minX = std::min({bl.x, br.x, tl.x, tr.x});
    float maxX = std::max({bl.x, br.x, tl.x, tr.x});
    float minY = std::min({bl.y, br.y, tl.y, tr.y});
    float maxY = std::max({bl.y, br.y, tl.y, tr.y});
    return math::Rect(minX, minY, maxX - minX, maxY - minY);
  }
};

} // namespace engine::components
