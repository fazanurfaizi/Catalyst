#pragma once

#include "engine/asset/core/Asset.hpp"
#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/AssetType.hpp"
#include <cstdint>
#include <filesystem>

namespace engine::resources {

class SoundAsset : public asset::Asset {
public:
  explicit SoundAsset(asset::AssetHandle handle) : asset::Asset(handle) {}

  static asset::AssetType StaticType() { return asset::AssetType::Sound; }
  virtual asset::AssetType Type() const override { return StaticType(); }

  bool IsEmpty() const noexcept { return sourcePath.empty(); }

public:
  std::filesystem::path sourcePath;
  std::uint32_t sampleRate = 0;
  std::uint32_t channels = 0;
  float durationSeconds = 0.0f;
};

} // namespace engine::resources
