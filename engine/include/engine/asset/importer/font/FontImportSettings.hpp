#pragma once

#include "engine/asset/core/AssetType.hpp"
#include "engine/asset/importer/ImportSettings.hpp"

#include <cstdint>
#include <string_view>

namespace engine::asset {

struct FontImportSettings : public ImportSettings {
  float pixelSize = 180.0f;
  int atlasWidth = 1024;
  int atlasHeight = 1024;
  std::uint32_t firstCodepoint = 32;
  std::uint32_t charCount = 95;

  AssetType GetAssetType() const override { return AssetType::Font; }

  std::string_view GetImporterId() const override { return "catalyst.font"; }

  std::uint32_t GetSchemaVersion() const override { return 1; }

  std::uint64_t ComputeHash() const override;

  json ToJson() const override;
  void FromJson(const json &j) override;
};

} // namespace engine::asset
