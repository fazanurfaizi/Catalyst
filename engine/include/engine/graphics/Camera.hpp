#pragma once

#include <glm/glm.hpp>

namespace engine {

struct WorldBounds {
  float minX = 0.0f;
  float maxX = 0.0f;
  float minY = 0.0f;
  float maxY = 0.0f;

  bool Overlaps(float aMinX, float aMaxX, float aMinY, float aMaxY) const {
    return aMinX <= maxX && aMaxX >= minX && aMinY <= maxY && aMaxY >= minY;
  }
};

class Camera {
public:
  Camera() = default;

  // Viewport
  void SetViewport(int width, int height);
  int ViewportWidth() const { return m_ViewportW; }
  int ViewportHeight() const { return m_ViewportH; }
  float AspectRatio() const;

  // Transform
  void SetPosition(const glm::vec3 &pos) { m_Position = pos; }
  void Move(const glm::vec3 &delta) { m_Position += delta; }
  const glm::vec3 &Position() const { return m_Position; }

  void SetZoom(float zoom);
  void ZoomBy(float factor);
  float Zoom() const { return m_Zoom; }

  void SetRotation(float radians);
  void RotateBy(float radians) { SetRotation(m_Rotation + radians); }
  float Rotation() const { return m_Rotation; }

  // Half-height of the visible world region at zoom 1.0.
  void SetOrthoSize(float halfHeight);
  float OrthoSize() const { return m_OrthoSize; }

  // Matrices
  glm::mat4 ViewMatrix() const;
  glm::mat4 ProjectionMatrix() const;
  glm::mat4 ViewProjectionMatrix() const;

  // Coordinate conversion
  // Screen coords: top-left origin, Y down.
  glm::vec2 WorldToScreen(const glm::vec2 &world) const;
  glm::vec2 ScreenToWorld(const glm::vec2 &screen) const;

  WorldBounds VisibleWorldBounds() const;

private:
  glm::vec3 m_Position{0.0f};
  float m_Zoom = 1.0f;
  float m_Rotation = 0.0f;   // radians
  float m_OrthoSize = 10.0f; // half-height in world units
  int m_ViewportW = 1280;
  int m_ViewportH = 720;
};

} // namespace engine
