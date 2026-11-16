#pragma once

#include "engine/asset/core/AssetType.hpp"
#include "engine/asset/importer/ImportSettings.hpp"

#include <cstdint>
#include <string_view>

namespace engine::asset {

struct ShaderImportSettings : public ImportSettings {
  AssetType GetAssetType() const override { return AssetType::Shader; }

  std::string_view GetImporterId() const override { return "catalyst.shader"; }

  std::uint32_t GetSchemaVersion() const override { return 1; }

  std::uint64_t ComputeHash() const override;

  json ToJson() const override;
  void FromJson(const json &j) override;
};

} // namespace engine::asset
