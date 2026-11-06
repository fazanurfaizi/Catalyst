#pragma once

#include "engine/asset/core/Asset.hpp"
#include "engine/asset/core/AssetType.hpp"
#include <filesystem>

namespace engine::audio {

class SoundAsset : public asset::Asset {
public:
  explicit SoundAsset(asset::AssetHandle handle) : asset::Asset(handle) {}
  virtual ~SoundAsset();

  static asset::AssetType StaticType() { return asset::AssetType::Sound; }
  virtual asset::AssetType Type() const override { return StaticType(); }

public:
  std::filesystem::path sourcePath;
  std::uint32_t sampleRate = 0;
  std::uint32_t channels = 0;
  std::uint32_t frameCount = 0;
};

} // namespace engine::audio
