#pragma once

#include "engine/asset/core/ImportedAssetData.hpp"
#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

namespace engine::asset {

struct GlyphCpu {
  glm::vec2 texCoords[4]; // TL, TR, BR, BL in UV space [0, 1]
  glm::vec2 positions[4]; // TL, TR, BR, BL quad vertices
  glm::vec2 size;
  float advance = 0.0f;
};

class FontImportResult : public ImportedAssetData {
public:
  AssetType GetAssetType() const override { return AssetType::Font; }

  std::uint32_t atlasWidth = 0;
  std::uint32_t atlasHeight = 0;
  std::vector<std::uint8_t> atlasPixels;
  std::vector<GlyphCpu> glyphs;
  std::uint32_t textHeight = 0;
  std::uint32_t firstCodepoint = 0;
};

} // namespace engine::asset
