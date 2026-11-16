#pragma once

#include <cstdint>
#include <functional>

#include <nlohmann/json.hpp>

namespace engine::ecs {

struct Entity {
  static constexpr std::uint32_t kInvalid = 0xFFFFFFFFu;

  std::uint32_t index = kInvalid;
  std::uint32_t generation = 0;

  [[nodiscard]] constexpr bool IsValid() const noexcept {
    return index != kInvalid;
  }
  [[nodiscard]] constexpr explicit operator bool() const noexcept {
    return IsValid();
  }

  bool operator==(const Entity &) const = default;
};

inline void to_json(nlohmann::json &j, const Entity &e) {
  j = nlohmann::json::object({
      {"index", e.index},
      {"generation", e.generation},
  });
}

inline void from_json(const nlohmann::json &j, Entity &e) {
  if (!j.is_object())
    return;
  e.index = j.value("index", decltype(e.index){0});
  e.generation = j.value("generation", decltype(e.generation){0});
}

} // namespace engine::ecs

namespace std {
template <> struct hash<engine::ecs::Entity> {
  std::size_t operator()(const engine::ecs::Entity &e) const noexcept {
    return hash<std::uint64_t>{}(
        (static_cast<std::uint64_t>(e.generation) << 32) | e.index);
  }
};
} // namespace std
