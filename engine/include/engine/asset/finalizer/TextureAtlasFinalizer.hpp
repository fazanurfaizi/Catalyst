#pragma once

#include "engine/asset/core/Asset.hpp"
#include "engine/asset/core/AssetHandle.hpp"
#include <memory>

namespace engine::asset {

class TextureAtlasImportResult;

class TextureAtlasFinalizer {
public:
  static std::shared_ptr<Asset> Finalize(AssetHandle handle,
                                         const TextureAtlasImportResult &data,
                                         bool eagerGpu);
};

} // namespace engine::asset
