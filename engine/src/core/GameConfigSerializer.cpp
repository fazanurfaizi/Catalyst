#include "engine/core/GameConfigSerializer.hpp"

#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/AssetType.hpp"
#include "engine/asset/persistence/AtomicFileWriter.hpp"
#include "engine/utils/Logger.hpp"

#include <filesystem>
#include <nlohmann/json.hpp>

#include <cstdio>
#include <fstream>

namespace engine::core {

using json = nlohmann::json;

namespace {

// FNV-1a 64-bit, high bit forced to 1 so the result is never Null().
// Deterministic: same relative path always yields the same handle, so
// hand-editing a config file does not silently invalidate the asset
// identity between runs.
engine::asset::AssetHandle DeriveHandleFromPath(const std::string &path) {
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

} // namespace

std::optional<GameConfig>
GameConfigSerializer::Load(const std::filesystem::path &file) {
  std::ifstream in(file);
  if (!in) {
    Logger::Error("[GameConfigSerializer] cannot open '%s'", file.c_str());
    return std::nullopt;
  }

  json doc;
  try {
    in >> doc;
  } catch (const json::exception &e) {
    Logger::Error("[GameConfigSerializer] JSON parse error in '%s': %s",
                  file.c_str(), e.what());
    return std::nullopt;
  }

  if (!doc.is_object()) {
    Logger::Error("[GameConfigSerializer] root is not an object in '%s'",
                  file.c_str());
    return std::nullopt;
  }

  if (!doc.contains("version") || !doc["version"].is_number_integer()) {
    Logger::Error("[GameConfigSerializer] missing/invalid 'version' in '%s'",
                  file.c_str());
    return std::nullopt;
  }
  const int version = doc["version"].get<int>();
  if (version != 1) {
    Logger::Error("[GameConfigSerializer] unsupported version %d in '%s'",
                  version, file.c_str());
    return std::nullopt;
  }

  GameConfig config;
  config.version = version;

  if (doc.contains("projectName") && doc["projectName"].is_string()) {
    config.projectName = doc["projectName"].get<std::string>();
  }

  // Resolve root relative to the config file's directory.
  std::filesystem::path configDir =
      std::filesystem::absolute(file).parent_path();
  if (doc.contains("root") && doc["root"].is_string()) {
    config.root = configDir / doc["root"].get<std::string>();
  } else {
    config.root = configDir;
  }
  config.configPath = std::filesystem::absolute(file);

  if (!doc.contains("assets") || !doc["assets"].is_array()) {
    Logger::Error("[GameConfigSerializer] missing 'assets' array in '%s'",
                  file.c_str());
    return std::nullopt;
  }

  for (std::size_t i = 0; i < doc["assets"].size(); ++i) {
    const json &entry = doc["assets"][i];

    if (!entry.is_object()) {
      Logger::Warn(
          "[GameConfigSerializer] asset[%zu] is not an object; skipped", i);
      continue;
    }
    if (!entry.contains("type") || !entry["type"].is_string()) {
      Logger::Warn("[GameConfigSerializer] asset[%zu] missing 'type'; skipped",
                   i);
      continue;
    }
    if (!entry.contains("path") || !entry["path"].is_string()) {
      Logger::Warn("[GameConfigSerializer] asset[%zu] missing 'path'; skipped",
                   i);
      continue;
    }

    const std::string typeStr = entry["type"].get<std::string>();
    const std::string pathStr = entry["path"].get<std::string>();

    const std::string nameStr =
        (entry.contains("name") && entry["name"].is_string())
            ? entry["name"].get<std::string>()
            : std::string();

    const engine::asset::AssetType type =
        engine::asset::AssetTypeFromString(typeStr);
    if (type == engine::asset::AssetType::None) {
      Logger::Warn(
          "[GameConfigSerializer] asset[%zu] unknown type '%s'; skipped", i,
          typeStr.c_str());
      continue;
    }

    engine::asset::AssetHandle handle = engine::asset::AssetHandle::Null();
    if (entry.contains("handle") && entry["handle"].is_string()) {
      handle = engine::asset::AssetHandle::FromString(
          entry["handle"].get<std::string>());
    }
    if (handle.IsNull()) {
      handle = DeriveHandleFromPath(pathStr);
    }

    engine::asset::AssetRecord record(handle, type,
                                      std::filesystem::path(pathStr), nameStr);

    record.settingsHash = entry.value("settingsHash", std::uint64_t{0});

    const std::filesystem::path absPath = config.root / record.path;
    if (!std::filesystem::exists(absPath)) {
      record.state = engine::asset::AssetRecordState::Missing;
      Logger::Warn("[GameConfigSerializer] asset '%s' missing on disk: %s",
                   nameStr.empty() ? pathStr.c_str() : nameStr.c_str(),
                   absPath.c_str());
    }

    if (entry.contains("dependencies") && entry["dependencies"].is_array()) {
      for (const auto &dep : entry["dependencies"]) {
        if (dep.is_string()) {
          auto handle =
              engine::asset::AssetHandle::FromString(dep.get<std::string>());
          if (!handle.IsNull()) {
            record.dependencies.push_back(handle);
          }
        }
      }
    }

    config.assets.emplace_back(std::move(record));
  }

  return config;
}

bool GameConfigSerializer::Save(const GameConfig &config,
                                const std::filesystem::path &file) {
  json doc;
  doc["version"] = config.version;

  if (!config.projectName.empty()) {
    doc["projectName"] = config.projectName;
  }

  std::error_code ec;
  const auto configDir = std::filesystem::absolute(file).parent_path();
  auto relativeRoot = std::filesystem::relative(config.root, configDir, ec);
  if (ec || relativeRoot.empty()) {
    relativeRoot = config.root;
  }
  doc["root"] = relativeRoot.generic_string();

  doc["assets"] = json::array();

  for (const auto &record : config.assets) {
    if (record.path.empty()) {
      Logger::Warn("[GameConfigSerializer] skipping asset with empty path");
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

    doc["assets"].push_back(std::move(entry));
  }

  if (!asset::AtomicFileWriter::Write(file, doc.dump(2) + "\n")) {
    Logger::Error("[GameConfigSerializer] rename failed: %s",
                  ec.message().c_str());
    return false;
  }

  return true;
}

} // namespace engine::core
