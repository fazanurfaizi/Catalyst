#pragma once

#include <cstdint>
namespace engine::asset {

enum class AssetLoadState : std::uint8_t {
  Unloaded = 0,
  Loading = 1,
  Loaded = 2,
  Uploading = 3,
  Ready = 4,
  Failed = 5,
};

const char *AssetLoadStateToString(AssetLoadState state) noexcept;

} // namespace engine::asset
