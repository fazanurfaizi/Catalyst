#pragma once

#include <algorithm>

namespace game {

struct Health {
  int current = 100;
  int max = 100;

  [[nodiscard]] bool IsDead() const noexcept { return current <= 0; }

  [[nodiscard]] float Normalized() const noexcept {
    return max > 0 ? static_cast<float>(current) / static_cast<float>(max)
                   : 0.0f;
  }

  void TakeDamage(int amount) noexcept {
    current = std::max(0, current - amount);
  }

  void Heal(int amount) noexcept { current = std::min(max, current + amount); }
};

} // namespace game
