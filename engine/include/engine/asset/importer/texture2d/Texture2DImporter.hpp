#pragma once

#include "engine/asset/importer/IAssetImporter.hpp"

namespace engine::asset {

class Texture2DImporter : public IAssetImporter {
public:
  std::string_view GetId() const override { return "catalyst.texture2d"; }

  AssetType GetAssetType() const override { return AssetType::Texture2D; }

  std::vector<std::string_view> GetSupportedExtensions() const override {
    return {".png"};
  }

  std::unique_ptr<ImportSettings> GetDefaultSettings() const override {
    return nullptr;
  }

  ImportResult<ImportedAssetDataPtr>
  Import(const ImportRequest &request) const override;
};

} // namespace engine::asset
