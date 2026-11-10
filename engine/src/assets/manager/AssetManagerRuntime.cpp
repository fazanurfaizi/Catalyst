#include "engine/asset/manager/AssetManagerRuntime.hpp"
#include "engine/asset/finalizer/AssetFinalizer.hpp"
#include "engine/utils/Logger.hpp"
#include <utility>

namespace engine::asset {

AssetManagerRuntime::AssetManagerRuntime(ImporterRegistry &registry,
                                         std::filesystem::path projectRoot)
    : m_registry(registry), m_projectRoot(std::move(projectRoot)) {}

void AssetManagerRuntime::AddRecord(AssetRecord record) {
  const AssetHandle handle = record.handle;
  const std::string name = record.name;

  auto [it, inserted] = m_records.emplace(handle, std::move(record));
  if (!inserted) {
    Logger::Error("[AssetManagerRuntime] duplicate handle %s ignored",
                  handle.ToString().c_str());
  }

  if (!name.empty()) {
    auto [nit, ninserted] = m_nameToHandle.emplace(name, handle);
    if (!ninserted) {
      Logger::Error("[AssetManagerRuntime] duplicate name '%s' ignored "
                    "(already maps to handle %s)",
                    name.c_str(), m_nameToHandle[name].ToString().c_str());
    }
  }
}

AssetHandle AssetManagerRuntime::FindByName(std::string_view name) const {
  auto it = m_nameToHandle.find(std::string(name));
  return it == m_nameToHandle.end() ? AssetHandle::Null() : it->second;
}

const AssetRecord *AssetManagerRuntime::Record(AssetHandle handle) const {
  auto it = m_records.find(handle);
  return it == m_records.end() ? nullptr : &it->second;
}

AssetType AssetManagerRuntime::Type(AssetHandle handle) const {
  const AssetRecord *record = Record(handle);
  return record ? record->type : AssetType::None;
}

bool AssetManagerRuntime::IsValid(AssetHandle handle) const {
  return Record(handle) != nullptr;
}

AssetLoadState AssetManagerRuntime::State(AssetHandle handle) const {
  if (!Record(handle))
    return AssetLoadState::Unloaded;
  return m_cache.State(handle);
}

const ImportError *AssetManagerRuntime::LastError(AssetHandle handle) const {
  return m_cache.LastError(handle);
}

std::shared_ptr<Asset> AssetManagerRuntime::AcquireRaw(AssetHandle handle) {
  if (auto loaded = m_cache.FindLoaded(handle)) {
    return loaded;
  }

  const AssetRecord *record = Record(handle);
  if (!record) {
    Logger::Error("[AssetManagerRuntime] unknown handle %s",
                  handle.ToString().c_str());
    m_cache.MarkFailed(
        handle, ImportError::Make(ImportError::Code::Internal,
                                  "No record registered for this handle"));
    return nullptr;
  }

  if (record->state != AssetRecordState::Valid) {
    m_cache.MarkFailed(
        handle,
        ImportError::Make(ImportError::Code::FileNotFound,
                          "Asset record is not Valid (Missing or Broken)")
            .WithPath(record->path));
    return nullptr;
  }

  // Find importer by extension
  IAssetImporter *importer =
      m_registry.FindForExtension(record->path.extension().string());
  if (!importer) {
    m_cache.MarkFailed(
        handle, ImportError::Make(ImportError::Code::UnsupportedFormat,
                                  "No importer registered for extension"));
    return nullptr;
  }

  // Import CPU
  m_cache.MarkLoading(handle);
  ImportRequest request{m_projectRoot / record->path};
  auto imported = importer->Import(request);
  if (imported.IsError()) {
    m_cache.MarkFailed(handle, imported.Error());
    return nullptr;
  }

  // Finalize GPU
  auto asset = AssetFinalizer::Finalize(handle, std::move(imported.Value()));
  if (!asset) {
    m_cache.MarkFailed(
        handle, ImportError::Make(ImportError::Code::Internal,
                                  "Finalize failed (see log for details)")
                    .WithPath(record->path));
    return nullptr;
  }

  m_cache.Insert(handle, asset);
  return asset;
}

void AssetManagerRuntime::Retry(AssetHandle handle) {
  if (m_cache.State(handle) != AssetLoadState::Failed)
    return;
  m_cache.Clear(handle);
  (void)AcquireRaw(handle);
}

void AssetManagerRuntime::Release(AssetHandle handle) { m_cache.Clear(handle); }

void AssetManagerRuntime::Reimport(AssetHandle handle) {
  m_cache.Clear(handle);
  (void)AcquireRaw(handle);
  ++m_reloadGeneration;
}

} // namespace engine::asset
