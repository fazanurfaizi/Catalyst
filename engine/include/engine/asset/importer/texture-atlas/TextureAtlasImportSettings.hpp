#pragma once

#include "engine/asset/importer/ImportSettings.hpp"

namespace engine::asset {

struct TextureAtlasImportSettings : public ImportSettings {
  int maxAtlasDimension = 4096;
  int maxSpriteDimension = 0;

  AssetType GetAssetType() const override { return AssetType::TextureAtlas; }

  std::string_view GetImporterId() const override {
    return "catalyst.texture_atlas";
  }

  std::uint32_t GetSchemaVersion() const override { return 1; }

  std::uint64_t ComputeHash() const override;

  json ToJson() const override;
  void FromJson(const json &j) override;
};

} // namespace engine::asset
