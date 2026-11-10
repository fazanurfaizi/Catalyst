#pragma once

#include "engine/asset/importer/ImportSettings.hpp"

namespace engine::asset {

struct Texture2DImportSettings : public ImportSettings {
  bool magFilterLinear = false;

  AssetType GetAssetType() const override { return AssetType::Texture2D; }

  std::string_view GetImporterId() const override {
    return "catalyst.texture2d";
  }

  std::uint32_t GetSchemaVersion() const override { return 1; }

  std::uint64_t ComputeHash() const override;

  json ToJson() const override;
  void FromJson(const json &j) override;
};

} // namespace engine::asset
