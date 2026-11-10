#pragma once

#include <cstdint>
#include <string>
namespace engine::asset {

enum class AssetType : std::uint16_t {
  None = 0,
  Font,
  Texture2D,
  TextureAtlas,
  Shader,
  Sound,
  Animation,
  Prefab,
  Scene,
};

/*
 * @brief Stable textual name. Used by serializers - never change existing
 * names.
 * */
const char *AssetTypeToString(AssetType type) noexcept;

/*
 * @brief Inverse of AssetTypeToString. Returns AssetType::None on unknown
 * input.
 * */
AssetType AssetTypeFromString(const std::string &type) noexcept;

} // namespace engine::asset
