#pragma once

#include "engine/asset/core/Asset.hpp"
#include "engine/asset/core/AssetHandle.hpp"

namespace engine::asset {

class ShaderImportResult;

class ShaderFinalizer {
public:
  static std::shared_ptr<Asset> Finalize(AssetHandle handle,
                                         const ShaderImportResult &data);
};

} // namespace engine::asset
