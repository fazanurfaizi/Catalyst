#include "engine/asset/persistence/SidecarSerializer.hpp"
#include "engine/asset/importer/IAssetImporter.hpp"
#include "engine/asset/persistence/AtomicFileWriter.hpp"
#include "engine/utils/Logger.hpp"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <system_error>
#include <utility>

namespace engine::asset {

using nlohmann::json;

namespace {

std::filesystem::path MetaPathFor(const std::filesystem::path &assetPath) {
  std::filesystem::path p = assetPath;
  p += ".meta.json";
  return p;
}

} // namespace

std::unique_ptr<ImportSettings>
LoadSidecarSettings(const std::filesystem::path &assetPath,
                    const IAssetImporter *importer, std::uint64_t *outHash) {
  if (outHash)
    *outHash = 0;
  if (!importer)
    return nullptr;

  auto settings = importer->GetDefaultSettings();
  if (!settings)
    return nullptr;

  auto finalize = [&]() -> std::unique_ptr<ImportSettings> {
    if (outHash)
      *outHash = settings->ComputeHash();
    return std::move(settings);
  };

  const auto metaPath = MetaPathFor(assetPath);

  if (!std::filesystem::exists(metaPath)) {
    return finalize();
  }

  std::ifstream in(metaPath);
  if (!in) {
    Logger::Warn("[Sidecar] cannot open '%s'... using default settings",
                 metaPath.c_str());
    return finalize();
  }

  json doc;
  try {
    in >> doc;
  } catch (const json::exception &e) {
    Logger::Error("[Sidecar] parse error in '%s': %s... using default settings",
                  metaPath.c_str(), e.what());
    return finalize();
  }

  if (!doc.is_object() || !doc.contains("importer") ||
      !doc["importer"].is_string()) {
    Logger::Error(
        "[Sidecar] missing 'importer' filed in '%s'... using default settings",
        metaPath.c_str());
    return finalize();
  }

  const std::string importerId = doc["importer"].get<std::string>();
  if (importerId != importer->GetId()) {
    Logger::Error("[Sidecar] importer mismatch in '%s' "
                  "(expected '%s', got '%s'... using default settings",
                  metaPath.c_str(), std::string(importer->GetId()).c_str(),
                  importerId.c_str());
    return finalize();
  }

  const std::uint32_t schemaVersion = doc.value("version", 1u);
  if (schemaVersion != settings->GetSchemaVersion()) {
    Logger::Warn("[Sidecar] schema version mismatch in '%s' "
                 "(expected %u, got %u)... using default settings",
                 metaPath.c_str(), settings->GetSchemaVersion(), schemaVersion);
    return finalize();
  }

  try {
    settings->FromJson(doc.value("settings", json::object()));
  } catch (const std::exception &e) {
    Logger::Error(
        "[Sidecar] invalid 'settings' in '%s': %s... using default settings",
        metaPath.c_str(), e.what());
    settings = importer->GetDefaultSettings();
    if (!settings)
      return nullptr;
  }

  return finalize();
}

bool SaveSidecarSettings(const std::filesystem::path &assetPath,
                         const IAssetImporter *importer,
                         const ImportSettings &settings) {
  if (!importer)
    return false;

  json doc;
  doc["importer"] = std::string(importer->GetId());
  doc["version"] = settings.GetSchemaVersion();
  doc["settings"] = settings.ToJson();

  const auto metaPath = MetaPathFor(assetPath);
  return AtomicFileWriter::Write(metaPath, doc.dump(2) + "\n");
}

bool SaveSidecarSettings(const std::filesystem::path &assetPath,
                         const IAssetImporter *importer) {
  if (!importer)
    return false;

  auto settings = importer->GetDefaultSettings();
  if (!settings)
    return false;

  return SaveSidecarSettings(assetPath, importer, *settings);
}

/** Writes default settings only when the sidecar does not exist yet. */
bool EnsureSidecarSettings(const std::filesystem::path &assetPath,
                           const IAssetImporter *importer) {
  const auto metaPath = MetaPathFor(assetPath);

  std::error_code ec;
  if (std::filesystem::exists(metaPath, ec))
    return true;

  return SaveSidecarSettings(assetPath, importer);
}

} // namespace engine::asset
