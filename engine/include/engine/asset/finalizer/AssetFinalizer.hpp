#pragma once

#include "engine/asset/core/Asset.hpp"
#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/ImportedAssetData.hpp"
#include <memory>
namespace engine::asset {

class AssetFinalizer {
public:
  static std::shared_ptr<Asset>
  Finalize(AssetHandle handle, ImportedAssetDataPtr data, bool eagerGpu = true);
};

} // namespace engine::asset
