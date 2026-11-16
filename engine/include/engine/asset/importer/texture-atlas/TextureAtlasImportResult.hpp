#pragma once

#include "engine/asset/core/ImportedAssetData.hpp"
#include "engine/resources/TextureAtlasAsset.hpp"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace engine::asset {

class TextureAtlasImportResult : public ImportedAssetData {
public:
  AssetType GetAssetType() const override { return AssetType::TextureAtlas; }

  std::uint32_t width = 0;
  std::uint32_t height = 0;
  std::vector<std::uint8_t> rgba8;
  std::vector<std::pair<std::string, resources::AtlasRegion>> regions;
};

} // namespace engine::asset
