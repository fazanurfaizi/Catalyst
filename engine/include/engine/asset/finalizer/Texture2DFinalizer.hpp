#pragma once

#include "engine/asset/core/Asset.hpp"
#include "engine/asset/core/AssetHandle.hpp"
#include <memory>

namespace engine::asset {

class Texture2DImportResult;

class Texture2DFinalizer {
public:
  static std::shared_ptr<Asset> Finalize(AssetHandle handle,
                                         const Texture2DImportResult &data,
                                         bool eagerGpu = true);
};

} // namespace engine::asset
