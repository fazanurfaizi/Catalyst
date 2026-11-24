#pragma once

#include "engine/assets/core/AssetType.hpp"
#include <memory>

namespace engine::asset {

class ImportedAssetData {
public:
  virtual ~ImportedAssetData() = default;
  virtual AssetType GetAssetType() const = 0;
};

using ImportedAssetDataPtr = std::unique_ptr<ImportedAssetData>;

} // namespace engine::asset
