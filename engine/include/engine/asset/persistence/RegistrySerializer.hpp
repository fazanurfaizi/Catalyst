#pragma once

#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/AssetRecord.hpp"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <string_view>
#include <vector>

namespace engine::asset {

class RegistrySerializer {
public:
  /** Serialize records to a JSON array. Records with empty path are skipped. */
  static nlohmann::json ToJson(const std::vector<AssetRecord> &records);

  /** Parse a JSON array into records. */
  static std::vector<AssetRecord>
  FromJson(const nlohmann::json &array,
           const std::filesystem::path &projectRoot = {});

  /** FNV-1a 64-bit, high bit forced to 1 so the result is never Null(). */
  static AssetHandle DeriveHandleFromPath(std::string_view path);
};

} // namespace engine::asset
