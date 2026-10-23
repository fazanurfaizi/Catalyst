#include "engine/scene/Scene.hpp"
#include <cassert>

namespace engine {

Entity Scene::CreateEntity(const std::string &name) {
  Entity entity;
  if (!m_Free.empty()) {
    entity = m_Free.back();
    m_Free.pop_back();
  } else {
    entity = static_cast<Entity>(m_Entries.size());
    m_Entries.emplace_back();
    m_Alive.push_back(1);
  }

  auto &entry = m_Entries[entity];
  entry.name = name;
  entry.transform = {};
  entry.sprite = {};
  entry.animator = {};
  entry.particle.reset();
  entry.emitter.reset();

  m_Alive[entity] = 1;
  ++m_LiveCount;
  return entity;
}

void Scene::DestroyEntity(Entity entity) {
  if (!IsAlive(entity))
    return;

  m_Alive[entity] = 0;
  --m_LiveCount;
  m_Free.push_back(entity);
}

void Scene::Update(double deltatime) {
  const float dt = static_cast<float>(deltatime);
  for (std::size_t i = 0; i < m_Entries.size(); ++i) {
    if (!m_Alive[i])
      continue;

    auto &animator = m_Entries[i].animator;

    if (!animator.IsActive())
      continue;

    animator.Advance(dt);

    const auto &frame = animator.clip->frames[animator.currentFrame];
    m_Entries[i].sprite.uvMin = frame.uvMin;
    m_Entries[i].sprite.uvMax = frame.uvMax;
  }
}

void Scene::Clear() {
  m_Entries.clear();
  m_Alive.clear();
  m_Free.clear();
  m_LiveCount = 0;
}

Entity Scene::FindEntity(const std::string &name) const {
  for (std::size_t i = 0; i < m_Entries.size(); ++i) {
    if (m_Alive[i] && m_Entries[i].name == name) {
      return static_cast<Entity>(i);
    }
  }
  return kInvalidEntity;
}

bool Scene::IsAlive(Entity entity) const {
  return entity < m_Entries.size() && m_Alive[entity] != 0;
}

const std::string &Scene::NameOf(Entity entity) const {
  assert(IsAlive(entity) && "NameOf: dead entity");
  return m_Entries[entity].name;
}

Transform &Scene::TransformOf(Entity entity) {
  assert(IsAlive(entity) && "TransformOf: dead entity");
  return m_Entries[entity].transform;
}

const Transform &Scene::TransformOf(Entity entity) const {
  assert(IsAlive(entity) && "TransformOf: dead entity");
  return m_Entries[entity].transform;
}

Sprite &Scene::SpriteOf(Entity entity) {
  assert(IsAlive(entity) && "SpriteOf: dead entity");
  return m_Entries[entity].sprite;
}

const Sprite &Scene::SpriteOf(Entity entity) const {
  assert(IsAlive(entity) && "SpriteOf: dead entity");
  return m_Entries[entity].sprite;
}

Animator &Scene::AnimatorOf(Entity entity) {
  assert(IsAlive(entity) && "AnimatorOf: dead entity");
  return m_Entries[entity].animator;
}

const Animator &Scene::AnimatorOf(Entity entity) const {
  assert(IsAlive(entity) && "AnimatorOf: dead entity");
  return m_Entries[entity].animator;
}

std::optional<Particle> &Scene::ParticleOf(Entity e) {
  assert(IsAlive(e) && "ParticleOf: dead entity");
  return m_Entries[e].particle;
}
const std::optional<Particle> &Scene::ParticleOf(Entity e) const {
  assert(IsAlive(e) && "ParticleOf: dead entity");
  return m_Entries[e].particle;
}

std::optional<ParticleEmitter> &Scene::EmitterOf(Entity e) {
  assert(IsAlive(e) && "EmitterOf: dead entity");
  return m_Entries[e].emitter;
}
const std::optional<ParticleEmitter> &Scene::EmitterOf(Entity e) const {
  assert(IsAlive(e) && "EmitterOf: dead entity");
  return m_Entries[e].emitter;
}
} // namespace engine
