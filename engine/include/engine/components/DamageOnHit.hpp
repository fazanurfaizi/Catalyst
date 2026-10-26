#pragma once

namespace engine::components {

// Attached to a projectile/trap. When the collider triggers with
// another entity, the CombatSystem applies this damage to the target.
struct DamageOnHit {
  int amount = 100;
  float knockback = 0.0f;
  bool destroyOnHit = true;
};

} // namespace engine::components
