#include "engine/asset/AssetManager.hpp"
#include "engine/asset/importer/ImporterRegistry.hpp"
#include "engine/asset/importer/animation/AnimationImporter.hpp"
#include "engine/asset/importer/font/FontImporter.hpp"
#include "engine/asset/importer/sound/SoundImporter.hpp"
#include "engine/asset/importer/texture2d/Texture2DImporter.hpp"

namespace engine::asset {

void AssetManager::RegisterDefaultImporters(ImporterRegistry &registry) {
  registry.Register(std::make_unique<Texture2DImporter>());
  registry.Register(std::make_unique<FontImporter>());
  registry.Register(std::make_unique<SoundImporter>());
  registry.Register(std::make_unique<AnimationImporter>());
}

} // namespace engine::asset
