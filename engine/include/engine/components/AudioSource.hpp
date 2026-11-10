#pragma once

#include "engine/asset/core/AssetHandle.hpp"

namespace engine::components {

struct AudioSource {
  asset::AssetHandle clip{};
  float volume = 1.0f;
  bool playOnSpawn = false;
  bool trigger = false;
  bool hasPlayedOnSpawn = false;
};

} // namespace engine::components
