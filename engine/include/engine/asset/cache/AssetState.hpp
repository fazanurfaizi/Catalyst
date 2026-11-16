#pragma once

#include <cstdint>
namespace engine::asset {

enum class AssetLoadState : std::uint8_t {
  Unloaded = 0,
  Loading = 1,
  Loaded = 2,
  Uploaded = 3,
  Failed = 4,
};

const char *AssetLoadStateToString(AssetLoadState state) noexcept;

} // namespace engine::asset
