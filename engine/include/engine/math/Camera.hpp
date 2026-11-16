#pragma once

#include "engine/math/Rect.hpp"

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace engine::math {

struct Camera {
  glm::mat4 viewProjection{1.0f};
  Rect visibleBounds{};
  float aspect = 1.0f;

  /// Build a camera from its view-projection matrix, deriving visible bounds.
  [[nodiscard]] static Camera FromViewProjection(const glm::mat4 &vp,
                                                 float aspect) noexcept {
    Camera cam;
    cam.viewProjection = vp;
    cam.aspect = aspect;
    cam.visibleBounds = BoundsFromViewProjection(vp);
    return cam;
  }

  /// World-space AABB covered by the NDC cube [-1,1]².
  [[nodiscard]] static Rect
  BoundsFromViewProjection(const glm::mat4 &vp) noexcept {
    const glm::mat4 invVp = glm::inverse(vp);

    auto corner = [&](float nx, float ny) {
      const glm::vec4 w = invVp * glm::vec4(nx, ny, 0.0f, 1.0f);
      return glm::vec2(w) / w.w;
    };

    const auto tl = corner(-1.0f, 1.0f);
    const auto tr = corner(1.0f, 1.0f);
    const auto bl = corner(-1.0f, -1.0f);
    const auto br = corner(1.0f, -1.0f);

    const float minX = std::min({bl.x, br.x, tl.x, tr.x});
    const float maxX = std::max({bl.x, br.x, tl.x, tr.x});
    const float minY = std::min({bl.y, br.y, tl.y, tr.y});
    const float maxY = std::max({bl.y, br.y, tl.y, tr.y});
    return Rect(minX, minY, maxX - minX, maxY - minY);
  }

  [[nodiscard]] glm::vec2 WorldToNdc(const glm::vec2 &world) const noexcept {
    const glm::vec4 clip = viewProjection * glm::vec4(world, 0.0f, 1.0f);
    if (clip.w == 0.0f)
      return glm::vec2(0.0f);
    return glm::vec2(clip) / clip.w;
  }

  [[nodiscard]] glm::vec2 NdcToWorld(const glm::vec2 &ndc) const noexcept {
    const glm::vec4 world =
        glm::inverse(viewProjection) * glm::vec4(ndc, 0.0f, 1.0f);
    if (world.w == 0.0f)
      return glm::vec2(0.0f);
    return glm::vec2(world) / world.w;
  }

  [[nodiscard]] glm::vec2
  WorldToScreen(const glm::vec2 &world,
                const glm::ivec2 &fbSize) const noexcept {
    const float w = static_cast<float>(fbSize.x > 0 ? fbSize.x : 1);
    const float h = static_cast<float>(fbSize.y > 0 ? fbSize.y : 1);
    const glm::vec2 ndc = WorldToNdc(world);
    return {(ndc.x * 0.5f + 0.5f) * w, (0.5f - ndc.y * 0.5f) * h};
  }

  [[nodiscard]] glm::vec2
  ScreenToWorld(const glm::vec2 &screen,
                const glm::ivec2 &fbSize) const noexcept {
    const float w = static_cast<float>(fbSize.x > 0 ? fbSize.x : 1);
    const float h = static_cast<float>(fbSize.y > 0 ? fbSize.y : 1);
    const float ndcX = (screen.x / w) * 2.0f - 1.0f;
    const float ndcY = 1.0f - (screen.y / h) * 2.0f;
    return NdcToWorld({ndcX, ndcY});
  }
};

} // namespace engine::math
