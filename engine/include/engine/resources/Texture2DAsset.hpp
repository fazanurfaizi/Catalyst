#pragma once

#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/AssetType.hpp"
#include "engine/resources/TextureAsset.hpp"
#include "engine/utils/Buffer.hpp"
#include <memory>

namespace engine::resources {

class Texture2DAsset : public TextureAsset {
public:
  explicit Texture2DAsset(asset::AssetHandle handle);

  static asset::AssetType StaticType() { return asset::AssetType::Texture2D; }
  asset::AssetType Type() const override { return StaticType(); }

  static std::shared_ptr<Texture2DAsset>
  Create(asset::AssetHandle handle, const TextureSpecification &spec,
         const Buffer &data, bool uploadImmediately = true);

  void SetData(Buffer data);

protected:
  bool HandleGpuUpload() const override;
};

} // namespace engine::resources
