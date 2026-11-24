#pragma once

#include "engine/assets/core/ImportedAssetData.hpp"

#include <string>

namespace engine::asset {

class ShaderImportResult : public ImportedAssetData {
public:
  AssetType GetAssetType() const override { return AssetType::Shader; }

  std::string vertexSource;
  std::string fragmentSource;
  std::string debugName;
};

} // namespace engine::asset
