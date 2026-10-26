#pragma once

namespace engine::components {

// Timer self-destruct
struct Lifetime {
  float remaining = 1.0f;
  bool destroyOnExpire = true;
};

} // namespace game
