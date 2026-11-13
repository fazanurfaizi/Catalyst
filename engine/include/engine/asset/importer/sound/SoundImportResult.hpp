#pragma once

#include "engine/asset/core/ImportedAssetData.hpp"
#include <cstdint>
#include <filesystem>

namespace engine::asset {

class SoundImportResult : public ImportedAssetData {
public:
  AssetType GetAssetType() const override { return AssetType::Sound; }

  std::filesystem::path absolutePath;
  std::uint32_t sampleRate = 0;
  std::uint32_t channels = 0;
  std::uint64_t frameCount = 0;
};
} // namespace engine::asset
