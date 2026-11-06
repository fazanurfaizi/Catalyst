#pragma once

#include "engine/asset/core/Asset.hpp"
#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/AssetType.hpp"
#include "engine/graphics/text/Font.hpp"
#include <cstdint>
#include <memory>

namespace engine::graphics {

class FontAsset : public asset::Asset {
public:
  explicit FontAsset(asset::AssetHandle handle) : asset::Asset(handle) {}

  static asset::AssetType StaticType() { return asset::AssetType::Font; }
  asset::AssetType Type() const override { return StaticType(); }

  std::shared_ptr<graphics::Font> font;
  std::uint32_t textHeight = 0;
};

} // namespace engine::graphics
