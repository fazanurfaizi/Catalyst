#pragma once

#include "engine/asset/core/AssetType.hpp"
#include "engine/asset/core/ImportRequest.hpp"
#include "engine/asset/core/ImportResult.hpp"
#include "engine/asset/core/ImportedAssetData.hpp"
#include "engine/asset/importer/ImportSettings.hpp"
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
