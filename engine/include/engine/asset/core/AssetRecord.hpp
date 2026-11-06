#pragma once

#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/AssetType.hpp"

#include <cstdint>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace engine::asset {

enum class AssetRecordState : std::uint8_t {
  Valid = 0,
  Missing = 1,
  Broken = 2,
};

const char *AssetRecordStateToString(AssetRecordState state) noexcept;
AssetRecordState AssetRecordStateFromString(const std::string &str) noexcept;

struct AssetRecord {
  AssetRecord() = delete;

  AssetRecord(AssetHandle h, AssetType t, std::filesystem::path p)
      : handle(h), type(t), path(std::move(p)) {}

  AssetHandle handle;
  AssetType type;
  std::filesystem::path path;

  AssetRecordState state = AssetRecordState::Valid;
  std::uint64_t settingsHash = 0;
  std::vector<AssetHandle> dependencies;
};

} // namespace engine::asset
