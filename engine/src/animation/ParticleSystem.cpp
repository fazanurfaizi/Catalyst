#include "engine/animation/ParticleSystem.hpp"

#include "engine/scene/Scene.hpp"

#include <cmath>
#include <random>
#include <vector>

#include <glm/gtc/constants.hpp>

namespace engine {

static float RandF(float min, float max) {
  static std::mt19937 gen(1337);
  std::uniform_real_distribution<float> dist(min, max);
  return dist(gen);
}

void ParticleSystem::Update(Scene &scene, float deltaTime) {
  std::vector<Entity> particles;
  std::vector<Entity> emitters;

  scene.ForEach([&](Entity e, const Transform &, const Sprite &) {
    if (scene.ParticleOf(e).has_value())
      particles.push_back(e);
    if (scene.EmitterOf(e).has_value())
      emitters.push_back(e);
  });

  for (Entity e : particles) {
    auto &part = *scene.ParticleOf(e);
    part.life -= deltaTime;

    if (part.life <= 0.0f) {
      scene.DestroyEntity(e);
      continue;
    }

    float t = 1.0f - (part.life / part.maxLife);

    scene.TransformOf(e).position += part.velocity * deltaTime;
    scene.TransformOf(e).scale =
        glm::vec2(glm::mix(part.startScale, part.endScale, t));
    scene.SpriteOf(e).color = glm::mix(part.startColor, part.endColor, t);
  }

  for (Entity e : emitters) {
    ParticleEmitter emitter = *scene.EmitterOf(e);
    const glm::vec2 emitPos = scene.TransformOf(e).position;

    emitter.timeEmitted += deltaTime;

    if (emitter.timeEmitted >= emitter.duration) {
      scene.DestroyEntity(e);
      continue;
    }

    emitter.emitAccumulator += deltaTime;
    float interval = 1.0f / emitter.emitRate;

    while (emitter.emitAccumulator >= interval) {
      emitter.emitAccumulator -= interval;

      Entity p = scene.CreateEntity("Particle");
      auto &t = scene.TransformOf(p);
      auto &s = scene.SpriteOf(p);
      auto &part = scene.ParticleOf(p).emplace();

      t.position = emitPos;

      s.texture = emitter.texture;
      s.layer = emitter.layer;
      s.uvMin = emitter.uvMin;
      s.uvMax = emitter.uvMax;

      part.maxLife = part.life = RandF(emitter.minLife, emitter.maxLife);
      part.startScale = emitter.minScale;
      part.endScale = emitter.maxScale;
      part.startColor = emitter.startColor;
      part.endColor = emitter.endColor;

      // Shoot in a random direction
      float angle = RandF(0.0f, glm::two_pi<float>());
      float speed = RandF(emitter.minSpeed, emitter.maxSpeed);
      part.velocity = glm::vec2(std::cos(angle), std::sin(angle)) * speed;
    }

    scene.EmitterOf(e)->emitAccumulator = emitter.emitAccumulator;
    scene.EmitterOf(e)->timeEmitted = emitter.timeEmitted;
  }
}

} // namespace engine
