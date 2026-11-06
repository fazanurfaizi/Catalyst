#pragma once

#include "engine/asset/core/Asset.hpp"
#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/AssetType.hpp"
#include "engine/graphics/assets/Texture2DAsset.hpp"
#include "engine/graphics/text/Font.hpp"
#include <cstdint>
#include <memory>
#include <vector>

namespace engine::graphics {

class FontAsset : public asset::Asset {
public:
  explicit FontAsset(asset::AssetHandle handle) : asset::Asset(handle) {}

  asset::AssetType Type() const override { return asset::AssetType::Font; }

  std::shared_ptr<Texture2DAsset> atlasTexutre;
  std::vector<Glyph> glyphs;
  std::uint32_t textHeight;
};

} // namespace engine::graphics
