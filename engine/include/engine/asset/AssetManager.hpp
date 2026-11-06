#pragma once

namespace engine::asset {

class ImporterRegistry;

class AssetManager {
public:
  static void RegisterDefaultImporters(ImporterRegistry &registry);
};

} // namespace engine::asset
