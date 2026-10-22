#include "engine/scene/Scene.hpp"
#include "engine/core/Logger.hpp"
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

bool Scene::IsAlive(Entity entity) const {
  return entity < m_Entries.size() && m_Alive[entity] != 0;
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
} // namespace engine
