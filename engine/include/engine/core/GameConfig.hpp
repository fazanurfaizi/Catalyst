#pragma once

#include "engine/asset/core/AssetRecord.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace engine::core {

struct GameConfig {
  int version = 1;
  std::string projectName;
  std::filesystem::path root;
  std::filesystem::path configPath;
  std::vector<engine::asset::AssetRecord> assets;
};

} // namespace engine::core
