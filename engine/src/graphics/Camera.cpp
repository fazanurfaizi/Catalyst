#include "engine/graphics/Camera.hpp"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

namespace engine {

void Camera::SetViewport(int width, int height) {
  m_ViewportW = std::max(1, width);
  m_ViewportH = std::max(1, height);
}

float Camera::AspectRatio() const {
  return static_cast<float>(m_ViewportW) / static_cast<float>(m_ViewportH);
}

void Camera::SetZoom(float zoom) { m_Zoom = std::max(zoom, 0.0001f); }

void Camera::ZoomBy(float factor) { SetZoom(m_Zoom * factor); }

void Camera::SetRotation(float radians) { m_Rotation = radians; }

void Camera::SetOrthoSize(float halfHeight) {
  m_OrthoSize = std::max(halfHeight, 0.0001f);
}

glm::mat4 Camera::ViewMatrix() const {
  // View = S(1/zoom) * R(-rotation) * T(-position)
  const glm::mat4 S = glm::scale(glm::mat4(1.0f),
                                 glm::vec3(1.0f / m_Zoom, 1.0f / m_Zoom, 1.0f));
  const glm::mat4 R =
      glm::rotate(glm::mat4(1.0f), -m_Rotation, glm::vec3(0, 0, 1));
  const glm::mat4 T = glm::translate(glm::mat4(1.0f), -m_Position);
  return S * R * T;
}

glm::mat4 Camera::ProjectionMatrix() const {
  const float halfH = m_OrthoSize;
  const float halfW = halfH * AspectRatio();
  return glm::ortho(-halfW, halfW, -halfH, halfH, -1000.0f, 1000.0f);
}

glm::mat4 Camera::ViewProjectionMatrix() const {
  return ProjectionMatrix() * ViewMatrix();
}

glm::vec2 Camera::WorldToScreen(const glm::vec2 &world) const {
  glm::vec4 clip = ViewProjectionMatrix() * glm::vec4(world, 0.0f, 1.0f);
  if (clip.w == 0.0f)
    return glm::vec2(0.0f);

  glm::vec3 ndc = glm::vec3(clip) / clip.w;
  // NDC (-1..1) -> screen (0..1) -> viewport pixels, flip Y for top-left origin
  float sx = (ndc.x * 0.5f + 0.5f) * m_ViewportW;
  float sy = (-ndc.y * 0.5f + 0.5f) * m_ViewportH;
  return glm::vec2(sx, sy);
}

glm::vec2 Camera::ScreenToWorld(const glm::vec2 &screen) const {
  // Screen -> NDC, flip Y back
  float ndcX = (screen.x / m_ViewportW) * 2.0f - 1.0f;
  float ndcY = -(screen.y / m_ViewportH) * 2.0f + 1.0f;
  glm::vec4 world =
      glm::inverse(ViewProjectionMatrix()) * glm::vec4(ndcX, ndcY, 0.0f, 1.0f);
  if (world.w == 0.0f)
    return glm::vec2(0.0f);
  return glm::vec2(world) / world.w;
}

WorldBounds Camera::VisibleWorldBounds() const {
  const float w = static_cast<float>(m_ViewportW);
  const float h = static_cast<float>(m_ViewportH);

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

} // namespace engine
