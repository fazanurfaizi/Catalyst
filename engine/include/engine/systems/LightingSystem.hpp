#pragma once

#include "engine/components/Light.hpp"
#include "engine/components/Transform.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/graphics/LightBuffer.hpp"
#include "engine/graphics/Renderer.hpp"
#include "engine/math/Math.hpp"

#include <glm/glm.hpp>

namespace engine::systems {

/**
 * Collects every light component in the registry into the renderer's
 * light buffer, then submits:
 * - UBOs (points / directional / spots)
 * - scalar uniforms (counts, ambient, view position)
 * - binds the light shader and restores the sprite program
 */
inline void LightingSystem(ecs::Registry &registry,
                           graphics::Renderer &renderer) {
  auto &out = renderer.MutableLightBuffer();
  out.pointCount = 0;
  out.directionalCount = 0;
  out.spotCount = 0;

  /**
   * Point lights
   */
  for (auto [e, t, l] :
       registry.View<components::Transform, components::PointLight>()) {
    if (out.pointCount >= graphics::LightBuffer::kMaxPoint) {
      Logger::Warn("[LightingSystem] point light limit (%d) reached",
                   graphics::LightBuffer::kMaxPoint);
      break;
    }
    auto &g = out.points[out.pointCount++];
    // @NOTE: for now use vec2 for position, set z to 0.0f
    // 2D world for now — z = 0, homogeneous w = 1
    g.position = glm::vec4(t.position.x, t.position.y, 0.0f, 1.0f);
    g.ambient = glm::vec4(l.ambient, 0.0f);
    g.diffuse = glm::vec4(l.diffuse, 0.0f);
    g.specular = glm::vec4(l.specular, 0.0f);
    g.attenuation = glm::vec4(l.constant, l.linear, l.quadratic, 0.0f);
  }

  /*
   * Directional lights
   */
  for (auto [e, l] : registry.View<components::DirectionalLight>()) {
    if (out.directionalCount >= graphics::LightBuffer::kMaxDirectional) {
      Logger::Warn("[LightingSystem] directional light limit (%d) reached",
                   graphics::LightBuffer::kMaxDirectional);
      break;
    }
    auto &g = out.directional[out.directionalCount++];
    g.direction =
        glm::vec4(math::safeNormalize(l.direction, {0.0f, -1.0f, 0.0f}), 0.0f);
    g.color =
        math::Color::fromVec4(l.color.toVec4() * l.intensity).withAlpha(1.0f);
  }

  /**
   * Spot lights
   */
  for (auto [e, t, l] :
       registry.View<components::Transform, components::SpotLight>()) {
    if (out.spotCount >= graphics::LightBuffer::kMaxSpot) {
      Logger::Warn("[LightingSystem] spot light limit (%d) reached",
                   graphics::LightBuffer::kMaxDirectional);
      break;
    }
    auto &g = out.spots[out.spotCount++];
    g.position = glm::vec4(t.position.x, t.position.y, 0.0f, 0.0f);
    g.direction =
        glm::vec4(math::safeNormalize(l.direction, {0.0f, -1.0f, 0.0f}), 0.0f);
    g.color =
        math::Color::fromVec4(l.color.toVec4() * l.intensity).withAlpha(1.0f);
    g.cone = glm::vec4(l.viewAngle, l.fadeAngle, 0.0f, 0.0f);

    const float r = (l.radius > 0.001f) ? l.radius : 15.0f;
    g.attenuation = glm::vec4(1.0f, 4.5f / r, 75.0f / (r * r), 0.0f);
  }
}

} // namespace engine::systems
