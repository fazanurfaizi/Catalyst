#include "engine/asset/persistence/RegistrySerializer.hpp"

#include "engine/asset/core/AssetType.hpp"
#include "engine/utils/Logger.hpp"

#include <cstdio>

namespace engine::asset {

using nlohmann::json;

// FNV-1a 64-bit, high bit forced to 1 so the result is never Null().
// Deterministic: same relative path always yields the same handle, so
// hand-editing a config file does not silently invalidate the asset
// identity between runs.
AssetHandle RegistrySerializer::DeriveHandleFromPath(std::string_view path) {
  std::uint64_t h = 1469598103934665603ULL;
  for (unsigned char c : path) {
    h ^= c;
    h *= 1099511628211ULL;
  }
  h |= (1ULL << 63);
  char buf[17];
  std::snprintf(buf, sizeof(buf), "%016llx",
                static_cast<unsigned long long>(h));
  return engine::asset::AssetHandle::FromString(buf);
}

json RegistrySerializer::ToJson(const std::vector<AssetRecord> &records) {
  json array = json::array();

  for (const auto &record : records) {
    if (record.path.empty()) {
      Logger::Warn("[RegistrySerializer] skipping asset with empty path");
      continue;
    }

    json entry;
    entry["type"] = engine::asset::AssetTypeToString(record.type);
    entry["path"] = record.path.generic_string();

    if (!record.name.empty()) {
      entry["name"] = record.name;
    }

    if (!record.handle.IsNull()) {
      entry["handle"] = record.handle.ToString();
    }

    entry["settingsHash"] = record.settingsHash;

    entry["dependencies"] = json::array();
    for (const auto &dep : record.dependencies) {
      if (!dep.IsNull()) {
        entry["dependencies"].push_back(dep.ToString());
      }
    }

    array.push_back(std::move(entry));
  }

  return array;
}

std::vector<AssetRecord>
RegistrySerializer::FromJson(const json &array,
                             const std::filesystem::path &projectRoot) {
  std::vector<AssetRecord> records;

  if (!array.is_array()) {
    Logger::Error("[RegistrySerializer] 'assets' is not an array");
    return records;
  }

  for (std::size_t i = 0; i < array.size(); ++i) {
    const json &entry = array[i];

    if (!entry.is_object()) {
      Logger::Warn("[RegistrySerializer] asset[%zu] is not an object; skipped",
                   i);
      continue;
    }
    if (!entry.contains("type") || !entry["type"].is_string()) {
      Logger::Warn("[RegistrySerializer] asset[%zu] missing 'type'; skipped",
                   i);
      continue;
    }
    if (!entry.contains("path") || !entry["path"].is_string()) {
      Logger::Warn("[RegistrySerializer] asset[%zu] missing 'path'; skipped",
                   i);
      continue;
    }

    const std::string typeStr = entry["type"].get<std::string>();
    const std::string pathStr = entry["path"].get<std::string>();

    if (pathStr.empty()) {
      Logger::Warn("[RegistrySerializer] asset[%zu] empty 'path'; skipped", i);
      continue;
    }

    const AssetType type = AssetTypeFromString(typeStr);
    if (type == AssetType::None) {
      Logger::Warn("[RegistrySerializer] asset[%zu] unknown type '%s'; skipped",
                   i, typeStr.c_str());
      continue;
    }

    const std::string nameStr =
        (entry.contains("name") && entry["name"].is_string())
            ? entry["name"].get<std::string>()
            : std::string();

    AssetHandle handle = AssetHandle::Null();
    if (entry.contains("handle") && entry["handle"].is_string()) {
      handle = AssetHandle::FromString(entry["handle"].get<std::string>());
    }
    if (handle.IsNull()) {
      handle = DeriveHandleFromPath(pathStr);
    }

    AssetRecord record(handle, type, std::filesystem::path(pathStr), nameStr);
    record.settingsHash = entry.value("settingsHash", std::uint64_t{0});

    if (!projectRoot.empty()) {
      const auto absPath = projectRoot / record.path;
      std::error_code ec;
      if (!std::filesystem::exists(absPath, ec)) {
        record.state = AssetRecordState::Missing;
        Logger::Warn("[RegistrySerializer] asset '%s' missing on disk: %s",
                     nameStr.empty() ? pathStr.c_str() : nameStr.c_str(),
                     absPath.c_str());
      }
    }

    if (entry.contains("dependencies") && entry["dependencies"].is_array()) {
      for (const auto &dep : entry["dependencies"]) {
        if (!dep.is_string())
          continue;

        const auto depHandle = AssetHandle::FromString(dep.get<std::string>());
        if (!depHandle.IsNull()) {
          record.dependencies.push_back(depHandle);
        }
      }
    }

    records.emplace_back(std::move(record));
  }

  return records;
}

} // namespace engine::asset
