#pragma once

#include <glm/glm.hpp>

namespace engine::math {

/// Axis-aligned bounding box, used for both UI hit-testing and sprite layout.
/// Coordinates are top-left origin with +Y pointing down (matches the camera).
struct Rect {
  float x = 0.0f;
  float y = 0.0f;
  float width = 0.0f;
  float height = 0.0f;

  constexpr Rect() = default;
  constexpr Rect(float x_, float y_, float width_, float height_)
      : x(x_), y(y_), width(width_), height(height_) {}

  static Rect FromCenter(const glm::vec2 &center, float w, float h) {
    return {center.x - w * 0.5f, center.y - h * 0.5f, w, h};
  }

  float Left() const { return this->x; }
  float Top() const { return this->y; }
  float Right() const { return this->x + this->width; }
  float Bottom() const { return this->y + this->height; }

  glm::vec2 Center() const { return {x + width * 0.5f, y + height * 0.5f}; }
  glm::vec2 Size() const { return {width, height}; }

  /// AABB point containment
  bool Contains(const glm::vec2 &p) const;

  /// AABB-vs-AABB overlap
  bool Intersects(const Rect &other) const;

  Rect Inset(float amount) const {
    return {x + amount, y + amount, width - amount * 2.0f,
            height - amount * 2.0f};
  }

  Rect expanded(float amount) const { return Inset(-amount); }
};

} // namespace engine::math
