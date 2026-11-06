#pragma once

#include "engine/asset/core/Asset.hpp"
#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/AssetType.hpp"
#include "engine/graphics/assets/TextureAsset.hpp"
#include "engine/utils/Buffer.hpp"
#include <memory>

namespace engine::graphics {

class Texture2DAsset : public asset::Asset {
public:
  explicit Texture2DAsset(asset::AssetHandle handle) : asset::Asset(handle) {}

  static asset::AssetType StaticType() { return asset::AssetType::Texture2D; }
  asset::AssetType Type() const override { return StaticType(); }

  static std::shared_ptr<Texture2DAsset>
  Create(asset::AssetHandle handle, const TextureSpecification &sepc,
         const Buffer &data) {
    return nullptr;
  }
};
} // namespace engine::graphics
