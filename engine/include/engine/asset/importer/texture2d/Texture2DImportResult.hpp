
#pragma once

#include "engine/asset/core/ImportedAssetData.hpp"
#include <cstdint>
#include <vector>

namespace engine::asset {

enum class TextureFormat : std::uint8_t {
  RGB8 = 0,
  RGBA8 = 1,
};

class Texture2DImportResult : public ImportedAssetData {
public:
  AssetType GetAssetType() const override { return AssetType::Texture2D; }

  std::uint32_t width = 0;
  std::uint32_t height = 0;
  TextureFormat format = TextureFormat::RGBA8;
  std::vector<std::uint8_t> pixelBytes;
};
} // namespace engine::asset
