#include "engine/asset/cache/AssetState.hpp"

namespace engine::asset {

const char *AssetLoadStateToString(AssetLoadState state) noexcept {
  switch (state) {
  case AssetLoadState::Unloaded:
    return "Unloaded";
  case AssetLoadState::Loading:
    return "Loading";
  case AssetLoadState::Loaded:
    return "Loaded";
  case AssetLoadState::Uploaded:
    return "Uploaded";
  case AssetLoadState::Failed:
    return "Failed";
  }
  return "Unknown";
}

} // namespace engine::asset
