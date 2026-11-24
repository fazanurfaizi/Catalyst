#pragma once

#include "engine/assets/core/AssetType.hpp"
#include "engine/assets/core/ImportRequest.hpp"
#include "engine/assets/core/ImportResult.hpp"
#include "engine/assets/core/ImportedAssetData.hpp"
#include "engine/assets/importer/ImportSettings.hpp"

#include <memory>
#include <string_view>
#include <vector>

namespace engine::asset {

class IAssetImporter {
public:
  virtual ~IAssetImporter() = default;

  virtual std::string_view GetId() const = 0;
  virtual AssetType GetAssetType() const = 0;
  virtual std::vector<std::string_view> GetSupportedExtensions() const = 0;
  virtual std::unique_ptr<ImportSettings> GetDefaultSettings() const = 0;
  virtual ImportResult<ImportedAssetDataPtr>
  Import(const ImportRequest &request) const = 0;
};

} // namespace engine::asset
