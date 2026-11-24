#include "engine/assets/finalizer/AssetFinalizer.hpp"

#include "engine/assets/core/Asset.hpp"
#include "engine/assets/core/AssetType.hpp"
#include "engine/assets/finalizer/AnimationFinalizer.hpp"
#include "engine/assets/finalizer/FontFinalizer.hpp"
#include "engine/assets/finalizer/ShaderFinalizer.hpp"
#include "engine/assets/finalizer/SoundFinalizer.hpp"
#include "engine/assets/finalizer/Texture2DFinalizer.hpp"
#include "engine/assets/finalizer/TextureAtlasFinalizer.hpp"
#include "engine/assets/importer/animation/AnimationImportResult.hpp"
#include "engine/assets/importer/font/FontImportResult.hpp"
#include "engine/assets/importer/shader/ShaderImportResult.hpp"
#include "engine/assets/importer/sound/SoundImportResult.hpp"
#include "engine/assets/importer/texture-atlas/TextureAtlasImportResult.hpp"
#include "engine/assets/importer/texture2d/Texture2DImportResult.hpp"
#include "engine/utils/Logger.hpp"

#include <memory>

namespace engine::asset {

std::shared_ptr<Asset> AssetFinalizer::Finalize(AssetHandle handle,
                                                ImportedAssetDataPtr data,
                                                bool eagerGpu) {
  if (!data) {
    Logger::Error("[AssetFinalizer] null import data for handle %s",
                  handle.ToString().c_str());
    return nullptr;
  }

  switch (data->GetAssetType()) {
  case AssetType::Texture2D:
    return Texture2DFinalizer::Finalize(
        handle, static_cast<const Texture2DImportResult &>(*data), eagerGpu);
  case AssetType::TextureAtlas:
    return TextureAtlasFinalizer::Finalize(
        handle, static_cast<const TextureAtlasImportResult &>(*data), eagerGpu);
  case AssetType::Font:
    return FontFinalizer::Finalize(
        handle, static_cast<const FontImportResult &>(*data), eagerGpu);
  case AssetType::Sound:
    return SoundFinalizer::Finalize(
        handle, static_cast<const SoundImportResult &>(*data), eagerGpu);
  case AssetType::Animation:
    return AnimationFinalizer::Finalize(
        handle, static_cast<const AnimationImportResult &>(*data), eagerGpu);
  case AssetType::Shader:
    return ShaderFinalizer::Finalize(
        handle, static_cast<const ShaderImportResult &>(*data), eagerGpu);
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
