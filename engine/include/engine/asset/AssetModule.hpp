#pragma once

namespace engine::asset {

class ImporterRegistry;

class AssetModule {
public:
  static void RegisterDefaultImporters(ImporterRegistry &registry);
};

} // namespace engine::asset
