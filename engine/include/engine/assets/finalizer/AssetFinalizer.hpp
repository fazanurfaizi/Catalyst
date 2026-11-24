#pragma once

#include "engine/assets/core/Asset.hpp"
#include "engine/assets/core/AssetHandle.hpp"
#include "engine/assets/core/ImportedAssetData.hpp"
#include <memory>

namespace engine::asset {

class AssetFinalizer {
public:
  static std::shared_ptr<Asset>
  Finalize(AssetHandle handle, ImportedAssetDataPtr data, bool eagerGpu = true);
};

} // namespace engine::asset
