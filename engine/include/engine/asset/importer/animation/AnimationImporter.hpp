#pragma once

#include "engine/asset/importer/IAssetImporter.hpp"

namespace engine::asset {

class AnimationImporter : public IAssetImporter {
public:
  std::string_view GetId() const override { return "catalyst.animation"; }

  AssetType GetAssetType() const override { return AssetType::Animation; }

  std::vector<std::string_view> GetSupportedExtensions() const override {
    return {".anim"};
  }

  std::unique_ptr<ImportSettings> GetDefaultSettings() const override {
    return nullptr;
  }

  ImportResult<ImportedAssetDataPtr>
  Import(const ImportRequest &request) const override;
};
} // namespace engine::asset
