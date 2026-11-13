#include "engine/asset/manager/AssetManagerEditor.hpp"
#include "engine/asset/importer/IAssetImporter.hpp"
#include "engine/asset/persistence/SidecarSerializer.hpp"
#include "engine/utils/Logger.hpp"
#include <filesystem>
#include <utility>

namespace engine::asset {

namespace {

bool EscapeRoot(const std::filesystem::path &relative) {
  if (relative.is_absolute())
    return true;

  const auto normalized = relative.lexically_normal();
  if (normalized.empty())
    return true;

  auto it = normalized.begin();
  if (it != normalized.end() && *it == "..")
    return true;

  return false;
}

} // namespace

AssetHandle
AssetManagerEditor::ImportAsset(const std::filesystem::path &relativePath,
                                std::string name) {
  if (EscapeRoot(relativePath)) {
    Logger::Error("[AssetManagerEditor] ImportAsset invalid path: '%s'",
                  relativePath.c_str());
    return AssetHandle::Null();
  }

  const auto normalized = relativePath.lexically_normal();
  const auto key = normalized.generic_string();

  if (const auto existing = FindByPath(key); !existing.IsNull()) {
    Logger::Info("[AssetManagerEditor] ImportAsset already registered: '%s'",
                 key.c_str());
    return existing;
  }

  const auto absPath = m_projectRoot / normalized;
  std::error_code ec;
  if (!std::filesystem::is_regular_file(absPath, ec)) {
    Logger::Error("[AssetManagerEditor] ImportAsset file not found: '%s'",
                  absPath.c_str());
    return AssetHandle::Null();
  }

  IAssetImporter *importer =
      m_registry.FindForExtension(normalized.extension().string());
  if (!importer) {
    Logger::Error(
        "[AssetManagerEditor] ImportAsset no importer for extension '%s'",
        normalized.extension().string().c_str());
    return AssetHandle::Null();
  }

  const AssetHandle handle = AssetHandle::Generate();
  AddRecord(AssetRecord(handle, importer->GetAssetType(), normalized,
                        std::move(name)));

  if (!EnsureSidecarSettings(absPath, importer)) {
    Logger::Warn("[AssetManagerEditor] sidecar not created for '%s'",
                 absPath.c_str());
  }

  Logger::Info("[AssetManagerEditor] imported '%s' as %s", key.c_str(),
               handle.ToString().c_str());

  return handle;
}

} // namespace engine::asset
