#pragma once

#include "engine/components/Particle.hpp"
#include "engine/components/ParticleEmitter.hpp"
#include "engine/components/Sprite.hpp"
#include "engine/components/Transform.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/ecs/Registry.hpp"

#include <cstdlib>

namespace engine::systems {

inline float RandRange(float lo, float hi) {
  return lo + (hi - lo) * (static_cast<float>(std::rand()) / RAND_MAX);
}

inline void ParticleSystem(ecs::Registry &reg, float dt) {
  for (auto [e, t, emitter] :
       reg.View<components::Transform, components::ParticleEmitter>()) {
    if (emitter.timeEmitted >= emitter.duration) {
      reg.DestroyDeferred(e);
      continue;
    }

    emitter.emitAccumulator += emitter.emitRate * dt;
    while (emitter.emitAccumulator >= 1.0f) {
      emitter.emitAccumulator -= 1.0f;

      const engine::ecs::Entity p = reg.Create();
      components::Transform pt;
      pt.position = t.position;

      components::Sprite ps;
      ps.texture = emitter.texture;
      ps.uvMin = emitter.uvMin;
      ps.uvMax = emitter.uvMax;
      ps.layer = emitter.layer;

      components::Particle pc;
      const float angle = RandRange(0.0f, 6.28318f);
      const float speed = RandRange(emitter.minSpeed, emitter.maxSpeed);
      pc.velocity = {std::cos(angle) * speed, std::sin(angle) * speed};
      pc.maxLife = RandRange(emitter.minLife, emitter.maxLife);
      pc.life = pc.maxLife;
      pc.startScale = RandRange(emitter.minScale, emitter.maxScale);
      pc.endScale = 0.0f;
      pc.startColor = emitter.startColor;
      pc.endColor = emitter.endColor;

      reg.Emplace<components::Transform>(p, pt);
      reg.Emplace<components::Sprite>(p, ps);
      reg.Emplace<components::Particle>(p, pc);
    }
    emitter.timeEmitted += dt;
  }

  reg.FlushDeferred();

  // Tick
  for (auto [e, t, p] :
       reg.View<components::Transform, components::Particle>()) {
    p.life -= dt;
    if (p.life <= 0.0f) {
      reg.DestroyDeferred(e);
      continue;
    }
    t.position += p.velocity * dt;
  }

  // Flush
  reg.FlushDeferred();
}

} // namespace engine::systems
