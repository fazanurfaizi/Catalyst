#pragma once

#include <cstdint>
#include <functional>

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

} // namespace engine::ecs

namespace std {
template <> struct hash<engine::ecs::Entity> {
  std::size_t operator()(const engine::ecs::Entity &e) const noexcept {
    return hash<std::uint64_t>{}(
        (static_cast<std::uint64_t>(e.generation) << 32) | e.index);
  }
};
} // namespace std
