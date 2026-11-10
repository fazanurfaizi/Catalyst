#include "engine/asset/core/AssetType.hpp"

namespace engine::asset {

const char *AssetTypeToString(AssetType type) noexcept {
  switch (type) {
  case AssetType::None:
    return "None";
  case AssetType::Font:
    return "Font";
  case AssetType::Texture2D:
    return "Texture2D";
  case AssetType::TextureAtlas:
    return "TextureAtlas";
  case AssetType::Shader:
    return "Shader";
  case AssetType::Sound:
    return "Sound";
  case AssetType::Animation:
    return "Animation";

  case AssetType::Prefab:
    return "Prefab";
  case AssetType::Scene:
    return "Scene";
  }
  return "None";
}

AssetType AssetTypeFromString(const std::string &type) noexcept {
  if (type == "None")
    return AssetType::None;
  if (type == "Font")
    return AssetType::Font;
  if (type == "Texture2D")
    return AssetType::Texture2D;
  if (type == "TextureAtlas")
    return AssetType::TextureAtlas;
  if (type == "Shader")
    return AssetType::Shader;
  if (type == "Sound")
    return AssetType::Sound;
  if (type == "Animation")
    return AssetType::Animation;
  if (type == "Prefab")
    return AssetType::Prefab;
  if (type == "Scene")
    return AssetType::Scene;
  return AssetType::None;
}

} // namespace engine::asset
