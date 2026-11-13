#pragma once

#include "engine/asset/importer/IAssetImporter.hpp"
#include "engine/asset/importer/shader/ShaderImportSettings.hpp"

namespace engine::asset {

class ShaderImporter : public IAssetImporter {
public:
  std::string_view GetId() const override { return "catalyst.shader"; }

  AssetType GetAssetType() const override { return AssetType::Shader; }

  std::vector<std::string_view> GetSupportedExtensions() const override {
    return {".vert.glsl", ".frag.glsl"};
  }

  std::unique_ptr<ImportSettings> GetDefaultSettings() const override {
    return std::make_unique<ShaderImportSettings>();
  }

  ImportResult<ImportedAssetDataPtr>
  Import(const ImportRequest &request) const override;
};
} // namespace engine::asset
