#pragma once

#include "engine/asset/importer/IAssetImporter.hpp"

namespace engine::asset {

class SoundImporter : public IAssetImporter {
public:
  std::string_view GetId() const override { return "catalyst.sound"; }

  AssetType GetAssetType() const override { return AssetType::Sound; }

  std::vector<std::string_view> GetSupportedExtensions() const override {
    return {".wav", ".mp3"};
  }

  std::unique_ptr<ImportSettings> GetDefaultSettings() const override {
    return nullptr;
  }

  ImportResult<ImportedAssetDataPtr>
  Import(const ImportRequest &request) const override;
};
} // namespace engine::asset
