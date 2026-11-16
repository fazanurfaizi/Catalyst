#pragma once

#include "engine/asset/importer/IAssetImporter.hpp"
#include "engine/asset/importer/texture-atlas/TextureAtlasImportSettings.hpp"

namespace engine::asset {

class TextureAtlasImporter : public IAssetImporter {
public:
  std::string_view GetId() const override { return "catalyst.texture_atlas"; }

  AssetType GetAssetType() const override { return AssetType::TextureAtlas; }

  std::vector<std::string_view> GetSupportedExtensions() const override {
    return {".atlas"};
  }

  std::unique_ptr<ImportSettings> GetDefaultSettings() const override {
    return std::make_unique<TextureAtlasImportSettings>();
  }

  ImportResult<ImportedAssetDataPtr>
  Import(const ImportRequest &request) const override;
};

} // namespace engine::asset
