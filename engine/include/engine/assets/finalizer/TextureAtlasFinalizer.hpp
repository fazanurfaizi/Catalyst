#pragma once

#include "engine/assets/core/Asset.hpp"
#include "engine/assets/core/AssetHandle.hpp"

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
