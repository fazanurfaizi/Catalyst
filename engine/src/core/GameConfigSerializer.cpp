#include "engine/core/GameConfigSerializer.hpp"

#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/AssetType.hpp"
#include "engine/asset/persistence/AtomicFileWriter.hpp"
#include "engine/asset/persistence/RegistrySerializer.hpp"
#include "engine/utils/Logger.hpp"

#include <filesystem>
#include <nlohmann/json.hpp>

#include <cstdio>
#include <fstream>

namespace engine::core {

using json = nlohmann::json;

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

  config.assets =
      asset::RegistrySerializer::FromJson(doc["assets"], config.root);

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

  doc["assets"] = asset::RegistrySerializer::ToJson(config.assets);

  if (!asset::AtomicFileWriter::Write(file, doc.dump(2) + "\n")) {
    Logger::Error("[GameConfigSerializer] rename failed: %s",
                  ec.message().c_str());
    return false;
  }

  return true;
}

} // namespace engine::core
