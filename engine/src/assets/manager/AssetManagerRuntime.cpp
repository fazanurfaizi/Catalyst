#include "engine/asset/manager/AssetManagerRuntime.hpp"
#include "engine/asset/finalizer/AssetFinalizer.hpp"
#include "engine/asset/persistence/SidecarSerializer.hpp"
#include "engine/utils/Logger.hpp"
#include <filesystem>
#include <mutex>
#include <utility>

namespace engine::asset {

AssetManagerRuntime::AssetManagerRuntime(ImporterRegistry &registry,
                                         std::filesystem::path projectRoot)
    : m_registry(registry), m_projectRoot(std::move(projectRoot)) {}

void AssetManagerRuntime::AddRecord(AssetRecord record) {
  std::lock_guard<std::mutex> lock(m_mutex);

  const AssetHandle handle = record.handle;
  const std::string name = record.name;

  auto [it, inserted] = m_records.emplace(handle, std::move(record));
  if (!inserted) {
    Logger::Error("[AssetManagerRuntime] duplicate handle %s ignored",
                  handle.ToString().c_str());
    return;
  }

  m_dirty = true;

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
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_nameToHandle.find(std::string(name));
  return it == m_nameToHandle.end() ? AssetHandle::Null() : it->second;
}

const AssetRecord *AssetManagerRuntime::Record(AssetHandle handle) const {
  std::lock_guard<std::mutex> lock(m_mutex);

  return RecordLocked(handle);
}

AssetType AssetManagerRuntime::Type(AssetHandle handle) const {
  std::lock_guard<std::mutex> lock(m_mutex);

  const AssetRecord *record = RecordLocked(handle);
  return record ? record->type : AssetType::None;
}

bool AssetManagerRuntime::IsValid(AssetHandle handle) const {
  std::lock_guard<std::mutex> lock(m_mutex);

  return RecordLocked(handle) != nullptr;
}

AssetLoadState AssetManagerRuntime::State(AssetHandle handle) const {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!RecordLocked(handle))
    return AssetLoadState::Unloaded;

  return m_cache.State(handle);
}

const ImportError *AssetManagerRuntime::LastError(AssetHandle handle) const {
  std::lock_guard<std::mutex> lock(m_mutex);

  return m_cache.LastError(handle);
}

std::shared_ptr<Asset> AssetManagerRuntime::AcquireRaw(AssetHandle handle) {
  return EnsureGpu(handle);
}

void AssetManagerRuntime::Retry(AssetHandle handle) {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_cache.State(handle) != AssetLoadState::Failed)
    return;
  m_cache.Clear(handle);
  (void)EnsureGpuLocked(handle);
}

void AssetManagerRuntime::Release(AssetHandle handle) {
  std::lock_guard<std::mutex> lock(m_mutex);

  m_cache.Clear(handle);
}

void AssetManagerRuntime::Reimport(AssetHandle handle) {
  std::lock_guard<std::mutex> lock(m_mutex);

  m_cache.Clear(handle);
  auto result = EnsureGpuLocked(handle);

  if (result) {
    ++m_reloadGeneration;
  } else {
    Logger::Warn("[Asset] reimport failed for %s, generation not bumped",
                 handle.ToString().c_str());
  }
}

std::uint64_t AssetManagerRuntime::GetReloadGeneration() const {
  std::lock_guard<std::mutex> lock(m_mutex);

  return m_reloadGeneration;
}

std::shared_ptr<Asset> AssetManagerRuntime::PreloadCpu(AssetHandle handle) {
  std::lock_guard<std::mutex> lock(m_mutex);

  return PreloadCpuLocked(handle);
}

std::shared_ptr<Asset> AssetManagerRuntime::EnsureGpu(AssetHandle handle) {
  std::lock_guard<std::mutex> lock(m_mutex);

  return EnsureGpuLocked(handle);
}

const AssetRecord *AssetManagerRuntime::RecordLocked(AssetHandle handle) const {
  auto it = m_records.find(handle);
  return it == m_records.end() ? nullptr : &it->second;
}

AssetRecord *AssetManagerRuntime::MutableRecordLocked(AssetHandle handle) {
  auto it = m_records.find(handle);
  return it == m_records.end() ? nullptr : &it->second;
}

std::optional<AssetRecord>
AssetManagerRuntime::RecordCopy(AssetHandle handle) const {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_records.find(handle);
  if (it == m_records.end())
    return std::nullopt;

  return it->second;
}

bool AssetManagerRuntime::RefreshSettingsHash(AssetHandle handle) {
  std::lock_guard<std::mutex> lock(m_mutex);

  AssetRecord *record = MutableRecordLocked(handle);
  if (!record)
    return false;

  if (record->settingsHash != 0)
    return false;

  IAssetImporter *importer =
      m_registry.FindForExtension(record->path.extension().string());
  if (!importer)
    return false;

  std::uint64_t hash = 0;
  (void)LoadSidecarSettings(m_projectRoot / record->path, importer, &hash);
  record->settingsHash = hash;
  return true;
}

AssetHandle
AssetManagerRuntime::FindByPath(std::string_view relativePath) const {
  std::lock_guard<std::mutex> lock(m_mutex);

  const std::filesystem::path needle =
      std::filesystem::path(relativePath).lexically_normal();

  for (const auto &[handle, record] : m_records) {
    if (record.path.lexically_normal() == needle) {
      return handle;
    }
  }

  return AssetHandle::Null();
}

std::vector<AssetRecord> AssetManagerRuntime::RecordsSnapshot() const {
  std::lock_guard<std::mutex> lock(m_mutex);

  std::vector<AssetRecord> out;
  out.reserve(m_records.size());
  for (const auto &[handle, record] : m_records) {
    out.push_back(record);
  }
  return out;
}

bool AssetManagerRuntime::IsDirty() const {
  std::lock_guard<std::mutex> lock(m_mutex);

  return m_dirty;
}

void AssetManagerRuntime::ClearDirty() {
  std::lock_guard<std::mutex> lock(m_mutex);

  m_dirty = false;
}

std::shared_ptr<Asset>
AssetManagerRuntime::PreloadCpuLocked(AssetHandle handle) {
  if (auto cached = m_cache.FindAny(handle)) {
    return cached;
  }

  AssetRecord *record = MutableRecordLocked(handle);
  if (!record) {
    Logger::Error("[AssetManagerRuntime] PreloadCpu unknown handle %s",
                  handle.ToString().c_str());
    m_cache.MarkFailed(handle, ImportError::Make(ImportError::Code::Internal,
                                                 "No record registered"));
    return nullptr;
  }

  if (record->state != AssetRecordState::Valid) {
    m_cache.MarkFailed(handle,
                       ImportError::Make(ImportError::Code::FileNotFound,
                                         "Asset record not Valid")
                           .WithPath(record->path));
    return nullptr;
  }

  // Importer lookup
  const std::filesystem::path absPath = m_projectRoot / record->path;
  IAssetImporter *importer =
      m_registry.FindForExtension(record->path.extension().string());
  if (!importer) {
    m_cache.MarkFailed(handle,
                       ImportError::Make(ImportError::Code::UnsupportedFormat,
                                         "No importer for extension"));
    return nullptr;
  }

  m_cache.MarkLoading(handle);

  std::uint64_t settingsHash = 0;
  auto settings = LoadSidecarSettings(absPath, importer, &settingsHash);

  if (record->settingsHash != 0 && record->settingsHash != settingsHash) {
    m_cache.Clear(handle);
    Logger::Info("[Asset] settings changed for %s, invalidating cache",
                 handle.ToString().c_str());
  }
  record->settingsHash = settingsHash;

  // Import CPU
  ImportRequest request{absPath, settings.get()};
  auto imported = importer->Import(request);
  if (imported.IsError()) {
    m_cache.MarkFailed(handle, imported.Error());
    return nullptr;
  }

  // Create CPU-only asset
  auto asset = AssetFinalizer::Finalize(handle, std::move(imported.Value()),
                                        /*eagerGpu=*/false);
  if (!asset) {
    m_cache.MarkFailed(handle, ImportError::Make(ImportError::Code::Internal,
                                                 "CreateCpu failed (see log)")
                                   .WithPath(record->path));
    return nullptr;
  }

  m_cache.Insert(handle, asset, AssetLoadState::Loaded);
  return asset;
}

std::shared_ptr<Asset>
AssetManagerRuntime::EnsureGpuLocked(AssetHandle handle) {
  auto asset = PreloadCpuLocked(handle);
  if (!asset)
    return nullptr;

  if (asset->IsGpuReady())
    return asset;

  if (!asset->EnsureGpu()) {
    m_cache.MarkFailed(handle, ImportError::Make(ImportError::Code::Internal,
                                                 "EnsureGpu failed (see log)"));
    return nullptr;
  }

  m_cache.MarkUploaded(handle);
  return asset;
}

} // namespace engine::asset
