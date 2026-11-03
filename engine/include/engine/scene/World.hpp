#pragma once

#include <string>

#include "engine/ecs/Registry.hpp"

#include <unordered_map>

namespace engine::scene {

class World {
public:
  World() = default;

  engine::ecs::Registry &Registry() { return m_registry; }

  engine::ecs::Entity CreateEntity(const std::string &name = "Entity");
  void DestroyEntity(engine::ecs::Entity e);

  engine::ecs::Entity FindEntity(const std::string &name) const;

private:
  engine::ecs::Registry m_registry;
  std::unordered_map<std::string, engine::ecs::Entity> m_nameToEntity;
};

} // namespace engine::scene
