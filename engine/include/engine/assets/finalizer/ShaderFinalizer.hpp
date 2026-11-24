#pragma once

#include "engine/assets/core/Asset.hpp"
#include "engine/assets/core/AssetHandle.hpp"

namespace engine::asset {

class ShaderImportResult;

class ShaderFinalizer {
public:
  static std::shared_ptr<Asset> Finalize(AssetHandle handle,
                                         const ShaderImportResult &data,
                                         bool eagerGpu = true);
};

} // namespace engine::asset
