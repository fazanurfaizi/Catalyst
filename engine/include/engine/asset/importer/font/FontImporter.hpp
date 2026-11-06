#pragma once

#include "engine/asset/importer/IAssetImporter.hpp"

namespace engine::asset {

class FontImporter : public IAssetImporter {
public:
  std::string_view GetId() const override { return "catalyst.font"; }

  AssetType GetAssetType() const override { return AssetType::Font; }

  std::vector<std::string_view> GetSupportedExtensions() const override {
    return {".ttf"};
  }

  std::unique_ptr<ImportSettings> GetDefaultSettings() const override {
    return nullptr;
  }

  ImportResult<ImportedAssetDataPtr>
  Import(const ImportRequest &request) const override;
};
} // namespace engine::asset
