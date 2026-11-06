#include "engine/asset/AssetManager.hpp"
#include "engine/asset/core/Asset.hpp"
#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/AssetType.hpp"
#include "engine/asset/core/ImportRequest.hpp"
#include "engine/asset/finalizer/AssetFinalizer.hpp"
#include "engine/asset/importer/ImporterRegistry.hpp"
#include "engine/utils/Logger.hpp"
#include <cassert>
// #include "game/KingdomRushGame.hpp"

int main(int /*argc*/, char ** /*argv*/) {
  engine::asset::ImporterRegistry registry;
  engine::asset::AssetManager::RegisterDefaultImporters(registry);

  struct Case {
    const char *ext;
    const char *path;
    engine::asset::AssetType expectedType;
  };
  const Case cases[] = {
      // {".png", "assets/test/hero.png", engine::asset::AssetType::Texture2D},
      // {".ttf", "assets/test/Roboto.ttf", engine::asset::AssetType::Font},
      {".wav", "assets/test/click.wav", engine::asset::AssetType::Sound},
      {".anim", "assets/test/walk.anim", engine::asset::AssetType::Animation},
  };

  for (const auto &c : cases) {
    auto *importer = registry.FindForExtension(c.ext);
    assert(importer && "no importer found");

    engine::asset::ImportRequest req{c.path};
    auto imported = importer->Import(req);
    if (imported.IsError()) {
      engine::Logger::Error("Import failed for '%s': %s", c.path,
                            imported.Error().ToString().c_str());
      assert(false && "cpu import failed");
      continue;
    }

    // Result type is the concrete subclass.
    assert(imported.Value()->GetAssetType() == c.expectedType);

    // Finalize into a live engine object stamped with a handle.
    engine::asset::AssetHandle handle = engine::asset::AssetHandle::Generate();
    std::shared_ptr<engine::asset::Asset> asset =
        engine::asset::AssetFinalizer::Finalize(handle,
                                                std::move(imported.Value()));
    assert(asset && "finalize failed");
    assert(asset->Handle() == handle && "handle not stamped");
    assert(asset->Type() == c.expectedType);

    engine::Logger::Info("OK: %s -> %s", c.ext, c.path);
  }
  // KingdomRushGame game;
  // game.Run();
  // return 0;
}
