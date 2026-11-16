#include "engine/scene/SceneSerializer.hpp"

#include "engine/ecs/ComponentSerializer.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/scene/Scene.hpp"
#include "engine/utils/Logger.hpp"

#include <cstddef>
#include <fstream>
#include <nlohmann/json.hpp>
#include <unordered_set>

namespace engine::scene {

bool SceneSerializer::Save(const scene::Scene &scene,
                           const std::string &filePath) {
  nlohmann::json jRoot = nlohmann::json::object();

  nlohmann::json jEntities = nlohmann::json::array();

  auto &reg = const_cast<scene::Scene &>(scene).Registry();
  const auto &serializers = ecs::ComponentRegistry::Instance().GetSerializers();

  // Gather unique alive entities across all registered component storages
  std::unordered_set<ecs::Entity> aliveEntities;
  for (const auto &[typeIdx, serializer] : serializers) {
    if (auto *storage = reg.TryStorageByType(typeIdx)) {
      auto *entities = storage->Entities();
      std::size_t size = storage->Size();
      for (std::size_t i = 0; i < size; ++i) {
        if (reg.IsAlive(entities[i])) {
          aliveEntities.insert(entities[i]);
        }
      }
    }
  }

  // Serialize each entity and its compiled components
  for (ecs::Entity e : aliveEntities) {
    nlohmann::json jEntity;
    jEntity["index"] = e.index;
    jEntity["generation"] = e.generation;

    nlohmann::json jComponents = nlohmann::json::object();
    for (const auto &[typeIdx, serializer] : serializers) {
      serializer->Serialize(reg, e, jComponents);
    }
    jEntity["components"] = jComponents;
    jEntities.push_back(jEntity);
  }

  jRoot["entities"] = jEntities;
  jRoot["version"] = 1;

  std::ofstream file(filePath);
  if (!file)
    return false;
  file << jRoot.dump(4);
  return true;
}

bool SceneSerializer::Load(scene::Scene &scene, const std::string &filePath) {
  std::ifstream file(filePath);
  if (!file)
    return false;

  nlohmann::json jRoot;
  try {
    file >> jRoot;
  } catch (nlohmann::json::exception &e) {
    Logger::Error("[SceneSerializer] failed to serialize json %s", e.what());
    return false;
  }

  if (!jRoot.contains("entities") || !jRoot["entities"].is_array()) {
    return false;
  }

  auto &reg = scene.Registry();
  const auto &serializers = ecs::ComponentRegistry::Instance().GetSerializers();

  {
    std::unordered_set<ecs::Entity> alive;
    for (const auto &[typeIdx, serializer] : serializers) {
      if (auto *storage = reg.TryStorageByType(typeIdx)) {
        auto *entities = storage->Entities();
        for (std::size_t i = 0; i < storage->Size(); ++i) {
          if (reg.IsAlive(entities[i]))
            alive.insert(entities[i]);
        }
      }
    }
    for (auto e : alive) {
      reg.Destroy(e);
    }
    reg.FlushDeferred();
  }

  for (const auto &jEntity : jRoot["entities"]) {
    ecs::Entity e = reg.Create();

    if (jEntity.contains("components") && jEntity["components"].is_object()) {
      const auto &jComponents = jEntity["components"];
      for (const auto &[typeIdx, serializer] : serializers) {
        serializer->Deserialize(scene.Registry(), e, jComponents);
      }
    }
  }

  scene.RebuildNameIndex();

  return true;
}

} // namespace engine::scene
