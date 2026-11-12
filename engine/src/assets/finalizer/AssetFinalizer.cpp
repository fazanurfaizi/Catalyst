#include "engine/asset/finalizer/AssetFinalizer.hpp"
#include "engine/asset/core/Asset.hpp"
#include "engine/asset/core/AssetType.hpp"
#include "engine/asset/finalizer/AnimationFinalizer.hpp"
#include "engine/asset/finalizer/FontFinalizer.hpp"
#include "engine/asset/finalizer/ShaderFinalizer.hpp"
#include "engine/asset/finalizer/SoundFinalizer.hpp"
#include "engine/asset/finalizer/Texture2DFinalizer.hpp"
#include "engine/asset/finalizer/TextureAtlasFinalizer.hpp"
#include "engine/asset/importer/animation/AnimationImportResult.hpp"
#include "engine/asset/importer/font/FontImportResult.hpp"
#include "engine/asset/importer/shader/ShaderImportResult.hpp"
#include "engine/asset/importer/sound/SoundImportResult.hpp"
#include "engine/asset/importer/texture-atlas/TextureAtlasImportResult.hpp"
#include "engine/asset/importer/texture2d/Texture2DImportResult.hpp"
#include "engine/utils/Logger.hpp"
#include <memory>

namespace engine::asset {

std::shared_ptr<Asset> AssetFinalizer::Finalize(AssetHandle handle,
                                                ImportedAssetDataPtr data) {
  if (!data) {
    Logger::Error("[AssetFinalizer] null import data for handle %s",
                  handle.ToString().c_str());
    return nullptr;
  }

  switch (data->GetAssetType()) {
  case AssetType::Texture2D:
    return Texture2DFinalizer::Finalize(
        handle, static_cast<const Texture2DImportResult &>(*data));
  case AssetType::TextureAtlas:
    return TextureAtlasFinalizer::Finalize(
        handle, static_cast<const TextureAtlasImportResult &>(*data));
  case AssetType::Font:
    return FontFinalizer::Finalize(
        handle, static_cast<const FontImportResult &>(*data));
  case AssetType::Sound:
    return SoundFinalizer::Finalize(
        handle, static_cast<const SoundImportResult &>(*data));
  case AssetType::Animation:
    return AnimationFinalizer::Finalize(
        handle, static_cast<const AnimationImportResult &>(*data));
  case AssetType::Shader:
    return ShaderFinalizer::Finalize(
        handle, static_cast<const ShaderImportResult &>(*data));
  case AssetType::Scene:
  case AssetType::Prefab:
  case AssetType::None:
  default:
    Logger::Error("[AssetFinalizer] no finalizer for type %s",
                  AssetTypeToString(data->GetAssetType()));
    return nullptr;
  }
}

} // namespace engine::asset
