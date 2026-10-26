#pragma once

#include "engine/ecs/Entity.hpp"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace game {

struct WorldBounds {
  float minX = 0.0f;
  float maxX = 0.0f;
  float minY = 0.0f;
  float maxY = 0.0f;

  [[nodiscard]] bool Overlaps(float aMinX, float aMaxX, float aMinY,
                              float aMaxY) const noexcept {
    return aMinX <= maxX && aMaxX >= minX && aMinY <= maxY && aMaxY >= minY;
  }

  [[nodiscard]] bool Contains(glm::vec2 p) const noexcept {
    return p.x >= minX && p.x <= maxX && p.y >= minY && p.y <= maxY;
  }
};

struct Camera2D {
  glm::vec2 position{0.0f};
  float zoom = 1.0f;
  float rotation = 0.0f; // radians
  float orthoSize = 10.0f;

  // Viewport (graphics)
  int viewportWidth = 1280;
  int viewportHeight = 720;

  // Game behavior
  bool active = true;
  engine::ecs::Entity followTarget{};
  float followSpeed = 8.0f;
  glm::vec2 followOffset{0.0f, 0.0f};

  void SetViewport(int w, int h) noexcept {
    viewportWidth = w;
    viewportHeight = h;
  }

  void SetPosition(glm::vec2 p) noexcept { position = p; }
  void Move(glm::vec2 d) noexcept { position += d; }
  void SetZoom(float z) noexcept { zoom = z > 1e-4f ? z : 1e-4f; }
  void ZoomBy(float f) noexcept { SetZoom(zoom * f); }
  void SetRotation(float r) noexcept { rotation = r; }
  void RotateBy(float r) noexcept { rotation += r; }

  [[nodiscard]] float AspectRatio() const noexcept {
    return viewportHeight > 0 ? static_cast<float>(viewportWidth) /
                                    static_cast<float>(viewportHeight)
                              : 1.0f;
  }

  [[nodiscard]] glm::mat4 ViewMatrix() const noexcept {
    glm::mat4 m(1.0f);
    m = glm::translate(m, glm::vec3(-position, 0.0f));
    if (rotation != 0.0f) {
      m = glm::rotate(m, -rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    }
    return m;
  }

  [[nodiscard]] glm::mat4 ProjectionMatrix() const noexcept {
    const float h = orthoSize / zoom;
    const float w = h * AspectRatio();
    return glm::ortho(-w, w, -h, h, -1.0f, 1.0f);
  }

  [[nodiscard]] glm::mat4 ViewProjectionMatrix() const noexcept {
    return ProjectionMatrix() * ViewMatrix();
  }

  [[nodiscard]] glm::vec2 WorldToScreen(const glm::vec2 &world) const noexcept {
    glm::vec4 clip = ViewProjectionMatrix() * glm::vec4(world, 0.0f, 1.0f);
    if (clip.w == 0.0f)
      return glm::vec2(0.0f);

    glm::vec2 ndc = glm::vec2(clip) / clip.w;
    // NDC (-1..1) -> screen (0..1) -> viewport pixels, flip Y for top-left
    // origin
    float sx = (ndc.x * 0.5f + 0.5f) * viewportWidth;
    float sy = 1.0f - (-ndc.y * 0.5f + 0.5f) * viewportHeight;
    return glm::vec2(sx, sy);
  }

  [[nodiscard]] glm::vec2
  ScreenToWorld(const glm::vec2 &screen) const noexcept {
    // Screen -> NDC, flip Y back
    float ndcX = (screen.x / viewportWidth) * 2.0f - 1.0f;
    float ndcY = 1.0f - (screen.y / viewportHeight) * 2.0f;
    glm::vec4 world = glm::inverse(ViewProjectionMatrix()) *
                      glm::vec4(ndcX, ndcY, 0.0f, 1.0f);
    if (world.w == 0.0f)
      return glm::vec2(0.0f);
    return glm::vec2(world) / world.w;
  }

  [[nodiscard]] WorldBounds VisibleWorldBounds() const noexcept {
    const float w = static_cast<float>(viewportWidth);
    const float h = static_cast<float>(viewportHeight);

    const auto tl = ScreenToWorld({0.0f, 0.0f});
    const auto tr = ScreenToWorld({w, 0.0f});
    const auto bl = ScreenToWorld({0.0f, h});
    const auto br = ScreenToWorld({w, h});

    WorldBounds b;
    b.minX = std::min({tl.x, tr.x, bl.x, br.x});
    b.maxX = std::max({tl.x, tr.x, bl.x, br.x});
    b.minY = std::min({tl.y, tr.y, bl.y, br.y});
    b.maxY = std::max({tl.y, tr.y, bl.y, br.y});
    return b;
  }
};

} // namespace game
