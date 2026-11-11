#pragma once

#include "engine/ecs/Entity.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/math/MathJson.hpp"

#include <field_reflection.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

namespace engine::ecs {

class IComponentSerializer {
public:
  virtual ~IComponentSerializer() = default;
  virtual void Serialize(const Registry &registry, Entity e,
                         nlohmann::json &j) const = 0;
  virtual void Deserialize(Registry &registry, Entity e,
                           const nlohmann::json &j) const = 0;
};

template <typename T>
  requires field_reflection::field_namable<T>
class ComponentSerializerImpl : public IComponentSerializer {
  void Serialize(const Registry &registry, Entity e,
                 nlohmann::json &j) const override {
    if (const T *comp = registry.TryGet<T>(e)) {
      nlohmann::json jComp;
      field_reflection::for_each_field(
          *comp, [&](std::string_view name, const auto &val) {
            jComp[std::string(name)] = val;
          });
      j[typeid(T).name()] = jComp;
    }
  }

  void Deserialize(Registry &registry, Entity e,
                   const nlohmann::json &j) const override {
    std::string typeKey = typeid(T).name();
    if (j.contains(typeKey)) {
      const auto &jComp = j[typeKey];
      T &comp = registry.Emplace<T>(e);
      field_reflection::for_each_field(
          comp, [&](std::string_view name, auto &val) {
            std::string fieldName(name);
            if (jComp.contains(fieldName)) {
              val = jComp[fieldName].get<std::decay_t<decltype(val)>>();
            }
          });
    }
  }
};

class ComponentRegistry {
public:
  static ComponentRegistry &Instance() {
    static ComponentRegistry instance;
    return instance;
  }

  template <typename T> void Register() {
    m_serializers[typeid(T)] = std::make_unique<ComponentSerializerImpl<T>>();
  }

  template <typename T> std::type_index GetTypeIndex() { return typeid(T); }

  const auto &GetSerializers() const { return m_serializers; }

private:
  ComponentRegistry() = default;
  std::unordered_map<std::type_index, std::unique_ptr<IComponentSerializer>>
      m_serializers;
};

} // namespace engine::ecs
