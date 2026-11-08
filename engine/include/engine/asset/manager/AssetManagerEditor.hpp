#pragma once

#include "engine/asset/manager/AssetManagerRuntime.hpp"
#include <filesystem>

namespace engine::asset {

class AssetManagerEditor : public AssetManagerRuntime {
public:
  using AssetManagerRuntime::AssetManagerRuntime;

  AssetHandle ImportAsset(const std::filesystem::path &relativePath,
                          std::string name = {});

  void Reimport(AssetHandle handle);
};

} // namespace engine::asset
