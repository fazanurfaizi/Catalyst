#include "engine/scene/Scene.hpp"
#include "engine/components/Name.hpp"
#include "engine/ecs/ComponentSerializer.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/utils/Logger.hpp"

namespace engine::scene {

ecs::Entity Scene::CreateEntity(std::string_view name) {
  auto e = m_registry.Create();
  auto &n = m_registry.Emplace<components::Name>(e);
  n.value = std::string(name);

  if (!name.empty()) {
    auto [it, inserted] = m_nameToEntity.try_emplace(n.value, e);
    if (!inserted) {
      Logger::Warn("[Scene] duplicate name '%s' (entity %u already owns it)",
                   n.value.c_str(), static_cast<unsigned>(it->second.index));
    }
  }

  return e;
}

std::optional<ecs::Entity> Scene::FindEntity(std::string_view name) {
  auto it = m_nameToEntity.find(name);
  if (it == m_nameToEntity.end())
    return std::nullopt;

  if (!m_registry.IsAlive(it->second)) {
    m_nameToEntity.erase(it);
    return std::nullopt;
  }

  return it->second;
}

bool Scene::DestroyEntity(ecs::Entity e) {
  if (!m_registry.IsAlive(e))
    return false;
  if (auto *n = m_registry.TryGet<components::Name>(e);
      n && !n->value.empty()) {
    auto it = m_nameToEntity.find(n->value);
    if (it != m_nameToEntity.end() && it->second == e)
      m_nameToEntity.erase(it);
  }
  m_registry.Destroy(e);
  return true;
}

void Scene::RebuildNameIndex() {
  m_nameToEntity.clear();

  auto typeIdx =
      ecs::ComponentRegistry::Instance().GetTypeIndex<components::Name>();
  auto *storage = m_registry.TryStorageByType(typeIdx);
  if (!storage)
    return;

  auto *entites = storage->Entities();
  const auto size = storage->Size();
  for (std::size_t i = 0; i < size; ++i) {
    auto e = entites[i];
    if (!m_registry.IsAlive(e))
      continue;
    auto *n = m_registry.TryGet<components::Name>(e);
    if (!n || n->value.empty())
      continue;
    m_nameToEntity.try_emplace(n->value, e);
  }
}

} // namespace engine::scene
