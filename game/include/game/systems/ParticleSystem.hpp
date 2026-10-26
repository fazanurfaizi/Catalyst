#pragma once

#include "engine/ecs/Entity.hpp"
#include "engine/ecs/Registry.hpp"
#include "game/components/Particle.hpp"
#include "game/components/ParticleEmitter.hpp"
#include "game/components/Sprite.hpp"
#include "game/components/Transform.hpp"

#include <cstdlib>

namespace game::systems {

inline float RandRange(float lo, float hi) {
  return lo + (hi - lo) * (static_cast<float>(std::rand()) / RAND_MAX);
}

inline void ParticleSystem(engine::ecs::Registry &reg, float dt) {
  for (auto [e, t, emitter] : reg.View<Transform, ParticleEmitter>()) {
    if (emitter.timeEmitted >= emitter.duration) {
      reg.Remove<ParticleEmitter>(e);
      continue;
    }

    emitter.emitAccumulator += emitter.emitRate * dt;
    while (emitter.emitAccumulator >= 1.0f) {
      emitter.emitAccumulator -= 1.0f;

      const engine::ecs::Entity p = reg.Create();
      Transform pt;
      pt.position = t.position;

      Sprite ps;
      ps.texture = emitter.texture;
      ps.uvMin = emitter.uvMin;
      ps.uvMax = emitter.uvMax;
      ps.layer = emitter.layer;

      Particle pc;
      const float angle = RandRange(0.0f, 6.28318f);
      const float speed = RandRange(emitter.minSpeed, emitter.maxSpeed);
      pc.velocity = {std::cos(angle) * speed, std::sin(angle) * speed};
      pc.maxLife = RandRange(emitter.minLife, emitter.maxLife);
      pc.life = pc.maxLife;
      pc.startScale = RandRange(emitter.minScale, emitter.maxScale);
      pc.endScale = 0.0f;
      pc.startColor = emitter.startColor;
      pc.endColor = emitter.endColor;

      reg.Emplace<Transform>(p, pt);
      reg.Emplace<Sprite>(p, ps);
      reg.Emplace<Particle>(p, pc);
    }
    emitter.timeEmitted += dt;
  }

  // Tick
  for (auto [e, t, p] : reg.View<Transform, Particle>()) {
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

} // namespace game::systems
