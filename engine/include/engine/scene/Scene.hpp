#pragma once

#include <functional>
#include <optional>
#include <string>

#include "engine/ecs/Entity.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/utils/StringHash.hpp"

#include <string_view>
#include <unordered_map>

namespace engine::scene {

class Scene {
public:
  Scene() = default;

  ecs::Registry &Registry() { return m_registry; }

  ecs::Entity CreateEntity(std::string_view name = "Entity");
  bool DestroyEntity(ecs::Entity e);
  std::optional<ecs::Entity> FindEntity(std::string_view name);

  void RebuildNameIndex();

private:
  ecs::Registry m_registry;
  std::unordered_map<std::string, ecs::Entity, StringHash, std::equal_to<>>
      m_nameToEntity;
};

} // namespace engine::scene
