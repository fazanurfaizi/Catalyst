#include "engine/scene/World.hpp"
#include "engine/components/Name.hpp"

namespace engine::scene {

engine::ecs::Entity World::CreateEntity(const std::string &name) {
  auto e = m_registry.Create();

  m_registry.Emplace<components::Name>(e);

  m_nameToEntity.try_emplace(name, e);
  return e;
}

} // namespace engine::scene
