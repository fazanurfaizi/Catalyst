#pragma once

#include "engine/asset/core/AssetType.hpp"
#include <filesystem>

namespace engine::asset {

struct AssetMetadata {
  AssetType type = AssetType::None;
  std::filesystem::path filePath;
};

} // namespace engine::asset
