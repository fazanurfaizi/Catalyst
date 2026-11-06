#pragma once

#include "engine/asset/core/Asset.hpp"
#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/AssetType.hpp"

namespace engine::graphics {

class SoundAsset : public asset::Asset {
public:
  explicit SoundAsset(asset::AssetHandle handle) : asset::Asset(handle) {}

  asset::AssetType Type() const override { return asset::AssetType::Sound; }
};

} // namespace engine::graphics
