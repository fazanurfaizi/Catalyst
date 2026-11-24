#include "engine/assets/AssetModule.hpp"

#include "engine/assets/builtin/BuiltinShaders.hpp"
#include "engine/assets/core/AssetRecord.hpp"
#include "engine/assets/core/AssetType.hpp"
#include "engine/assets/importer/ImporterRegistry.hpp"
#include "engine/assets/importer/animation/AnimationImporter.hpp"
#include "engine/assets/importer/font/FontImporter.hpp"
#include "engine/assets/importer/shader/ShaderImporter.hpp"
#include "engine/assets/importer/sound/SoundImporter.hpp"
#include "engine/assets/importer/texture-atlas/TextureAtlasImporter.hpp"
#include "engine/assets/importer/texture2d/Texture2DImporter.hpp"
#include "engine/assets/manager/IAssetManager.hpp"
#include "engine/utils/Logger.hpp"

namespace engine::asset {

void AssetModule::RegisterDefaultImporters(ImporterRegistry &registry) {
  if (registry.GetImporterCount() != 0) {
    Logger::Warn("[AssetModule] default importers already registered, skipped");
    return;
  }

  registry.Register(std::make_unique<Texture2DImporter>());
  registry.Register(std::make_unique<TextureAtlasImporter>());
  registry.Register(std::make_unique<FontImporter>());
  registry.Register(std::make_unique<SoundImporter>());
  registry.Register(std::make_unique<ShaderImporter>());
  registry.Register(std::make_unique<AnimationImporter>());
}

void AssetModule::RegisterBuiltinAssets(IAssetManager &assets) {
  for (const auto &def : builtin::ShaderDefs()) {
    AssetRecord record(builtin::HandleOf(def.name), AssetType::Shader,
                       builtin::PathOf(def.name), std::string(def.name));
    record.builtin = true;
    assets.AddRecord(std::move(record));
  }
}

} // namespace engine::asset
