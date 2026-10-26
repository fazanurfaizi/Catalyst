#pragma once

namespace game {

// Timer self-destruct
struct Lifetime {
  float remaining = 1.0f;
  bool destroyOnExpire = true;
};

} // namespace game
