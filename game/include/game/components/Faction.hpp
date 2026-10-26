#pragma once

#include <cstdint>

namespace game {

enum class FactionId : std::uint8_t {
  Neutral = 0,
  Player = 1,
  Enemy = 2,
  Ally = 3,
};

struct Faction {
  FactionId id = FactionId::Neutral;

  [[nodiscard]] bool HostileTo(Faction other) const noexcept {
    if (id == FactionId::Neutral || other.id == FactionId::Neutral)
      return false;
    return (id == FactionId::Player && other.id == FactionId::Enemy) ||
           (id == FactionId::Enemy && other.id == FactionId::Player);
  }
};

} // namespace game
