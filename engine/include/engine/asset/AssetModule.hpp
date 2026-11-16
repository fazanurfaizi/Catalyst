#pragma once

namespace engine::asset {

class ImporterRegistry;
class IAssetManager;

class AssetModule {
public:
  static void RegisterDefaultImporters(ImporterRegistry &registry);

  static void RegisterBuiltinAssets(IAssetManager &assets);
};

} // namespace engine::asset
