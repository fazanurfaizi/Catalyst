#pragma once

#include "engine/ecs/Entity.hpp"
#include "engine/math/Rect.hpp"
#include "engine/platform/Display.hpp"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace engine::components {

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

  const platform::Display *display = nullptr;

  void SetDisplay(const platform::Display *d) noexcept { display = d; }

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

  [[nodiscard]] glm::ivec2 ResolvedSize() const noexcept {
    if (display) {
      const auto fb = display->FramebufferSize();
      if (fb.x > 0 && fb.y > 0)
        return fb;
    }
    return {viewportWidth, viewportHeight};
  }

  [[nodiscard]] float AspectRatio() const noexcept {
    const auto s = ResolvedSize();
    return s.y > 0 ? static_cast<float>(s.x) / static_cast<float>(s.y) : 1.0f;
  }

  [[nodiscard]] glm::mat4 ViewMatrix() const noexcept {
    glm::mat4 m(1.0f);
    if (rotation != 0.0f) {
      m = glm::rotate(m, -rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    }
    m = glm::translate(m, glm::vec3(-position, 0.0f));
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
    const auto size = ResolvedSize();
    glm::vec4 clip = ViewProjectionMatrix() * glm::vec4(world, 0.0f, 1.0f);
    if (clip.w == 0.0f)
      return glm::vec2(0.0f);
    glm::vec2 ndc = glm::vec2(clip) / clip.w;
    // NDC -> screen pixels, origin top-left (Y flip diperbaiki)
    const float sx = (ndc.x * 0.5f + 0.5f) * float(size.x);
    const float sy = (0.5f - ndc.y * 0.5f) * float(size.y);
    return {sx, sy};
  }

  [[nodiscard]] glm::vec2
  ScreenToWorld(const glm::vec2 &screen) const noexcept {
    // Screen -> NDC, flip Y back
    const auto size = ResolvedSize();
    const float w = float(size.x > 0 ? size.x : 1);
    const float h = float(size.y > 0 ? size.y : 1);
    const float ndcX = (screen.x / w) * 2.0f - 1.0f;
    const float ndcY = 1.0f - (screen.y / h) * 2.0f;
    glm::vec4 world = glm::inverse(ViewProjectionMatrix()) *
                      glm::vec4(ndcX, ndcY, 0.0f, 1.0f);
    if (world.w == 0.0f)
      return glm::vec2(0.0f);
    return glm::vec2(world) / world.w;
  }

  [[nodiscard]] glm::vec2
  ScreenToWorldFromCursor(const glm::vec2 &windowCursor) const noexcept {
    if (display)
      return ScreenToWorld(display->CursorToFramebuffer(windowCursor));
    return ScreenToWorld(windowCursor); // fallback legacy
  }

  [[nodiscard]] math::Rect VisibleWorldBounds() const noexcept {
    const auto size = ResolvedSize();
    const float w = static_cast<float>(size.x);
    const float h = static_cast<float>(size.y);

    const auto tl = ScreenToWorld({0.0f, 0.0f});
    const auto tr = ScreenToWorld({w, 0.0f});
    const auto bl = ScreenToWorld({0.0f, h});
    const auto br = ScreenToWorld({w, h});

    float minX = std::min({tl.x, tr.x, bl.x, br.x});
    float maxX = std::max({tl.x, tr.x, bl.x, br.x});
    float minY = std::min({tl.y, tr.y, bl.y, br.y});
    float maxY = std::max({tl.y, tr.y, bl.y, br.y});
    return math::Rect(minX, minY, maxX - minX, maxY - minY);
  }
};

} // namespace engine::components
