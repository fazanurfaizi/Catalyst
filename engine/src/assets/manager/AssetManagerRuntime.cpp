#include "engine/assets/manager/AssetManagerRuntime.hpp"

#include "engine/assets/builtin/BuiltinShaders.hpp"
#include "engine/assets/core/AssetRecord.hpp"
#include "engine/assets/finalizer/AssetFinalizer.hpp"
#include "engine/assets/persistence/SidecarSerializer.hpp"
#include "engine/utils/Logger.hpp"

#include <exception>
#include <filesystem>
#include <mutex>
#include <string>
#include <utility>

namespace engine::asset {

AssetManagerRuntime::AssetManagerRuntime(ImporterRegistry &registry,
                                         std::filesystem::path projectRoot)
    : m_registry(registry), m_projectRoot(std::move(projectRoot)) {}

void AssetManagerRuntime::AddRecord(AssetRecord record) {
  std::lock_guard<std::mutex> lock(m_mutex);

  const AssetHandle handle = record.handle;
  const std::string name = record.name;
  const bool isBuiltin = record.builtin;

  auto [it, inserted] = m_records.emplace(handle, std::move(record));
  if (!inserted) {
    Logger::Error("[AssetManagerRuntime] duplicate handle %s ignored",
                  handle.ToString().c_str());
    return;
  }

  m_dirty = true;

  if (name.empty()) {
    return;
  }

  auto [nit, ninserted] = m_nameToHandle.emplace(name, handle);
  if (ninserted) {
    return;
  }

  const AssetRecord *owner = RecordLocked(nit->second);
  const bool ownerBuiltin = owner && owner->builtin;

  if (!isBuiltin && ownerBuiltin) {
    Logger::Info("[AssetManagerRuntime] project asset '%s' overrides "
                 "engine built-in",
                 name.c_str());
    nit->second = handle;
    return;
  }

  if (isBuiltin && owner && !owner->builtin) {
    return;
  }

  Logger::Error("[AssetManagerRuntime] duplicate name '%s' ignored "
                "(already maps to handle %s)",
                name.c_str(), m_nameToHandle[name].ToString().c_str());
}

AssetHandle AssetManagerRuntime::FindByName(std::string_view name) const {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_nameToHandle.find(std::string(name));
  return it == m_nameToHandle.end() ? AssetHandle::Null() : it->second;
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

const std::optional<ImportError>
AssetManagerRuntime::LastError(AssetHandle handle) const {
  std::lock_guard<std::mutex> lock(m_mutex);

  return m_cache.LastError(handle);
}

std::shared_ptr<Asset> AssetManagerRuntime::AcquireRaw(AssetHandle handle) {
  return EnsureGpu(handle);
}

void AssetManagerRuntime::Retry(AssetHandle handle) {
  {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_cache.State(handle) != AssetLoadState::Failed)
      return;
    m_cache.Clear(handle);
  }
  (void)EnsureGpu(handle);
}

void AssetManagerRuntime::Release(AssetHandle handle) {
  std::shared_ptr<Asset> asset;
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    asset = m_cache.FindAny(handle);
    m_cache.Clear(handle);
  }

  if (asset) {
    asset->ReleaseGpu();
  }
}

void AssetManagerRuntime::Reimport(AssetHandle handle) {
  std::shared_ptr<Asset> old;

  {
    std::lock_guard<std::mutex> lock(m_mutex);
    old = m_cache.FindAny(handle);
    m_cache.Clear(handle);
  }
  old.reset();

  auto result = EnsureGpu(handle);
  if (!result) {
    Logger::Warn("[Asset] reimport failed for %s, generation not bumped",
                 handle.ToString().c_str());
  }

  std::lock_guard<std::mutex> lock(m_mutex);
  ++m_reloadGeneration;
}

std::uint64_t AssetManagerRuntime::GetReloadGeneration() const {
  std::lock_guard<std::mutex> lock(m_mutex);

  return m_reloadGeneration;
}

std::shared_ptr<Asset> AssetManagerRuntime::PreloadCpu(AssetHandle handle) {
  std::unique_lock<std::mutex> lock(m_mutex);

  if (auto cached = m_cache.FindAny(handle)) {
    return cached;
  }

  while (m_inflight.count(handle) != 0) {
    m_cv.wait(lock);
    if (auto cached = m_cache.FindAny(handle))
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

  const bool isBuiltin = record->builtin;
  const std::string builtinName = record->name;

  // Importer lookup
  IAssetImporter *importer = nullptr;
  if (!isBuiltin) {
    importer = m_registry.FindForExtension(record->path.extension().string());
    if (!importer) {
      m_cache.MarkFailed(handle,
                         ImportError::Make(ImportError::Code::UnsupportedFormat,
                                           "No importer for extension"));
      return nullptr;
    }
  }

  const std::filesystem::path absPath =
      isBuiltin ? std::filesystem::path{} : m_projectRoot / record->path;
  std::uint64_t previousHash = record->settingsHash;

  m_inflight.insert(handle);
  m_cache.MarkLoading(handle);
  lock.unlock();

  std::uint64_t settingsHash = 0;
  std::shared_ptr<Asset> asset;
  std::optional<ImportError> failure;

  try {
    if (isBuiltin) {
      asset = builtin::CreateAsset(handle, builtinName);
      if (!asset) {
        failure = ImportError::Make(ImportError::Code::Internal,
                                    "Built-in asset source missing")
                      .WithPath(builtin::PathOf(builtinName));
      }
    } else {

      auto settings = LoadSidecarSettings(absPath, importer, &settingsHash);

      if (previousHash != 0 && previousHash != settingsHash) {
        Logger::Info("[AssetManagerRuntime] settings changed for %s, reloading",
                     handle.ToString().c_str());
      }

      ImportRequest request{absPath, settings.get()};
      auto imported = importer->Import(request);
      if (imported.IsError()) {
        failure = imported.Error();
      } else {
        asset = AssetFinalizer::Finalize(handle, std::move(imported.Value()),
                                         /*eagerGpu=*/false);

        if (!asset) {
          failure = ImportError::Make(ImportError::Code::Internal,
                                      "CreateCPU failed (see log)")
                        .WithPath(absPath);
        }
      }
    }
  } catch (const std::exception &e) {
    failure = ImportError::Make(ImportError::Code::Internal,
                                std::string("Importer threw: ") + e.what())
                  .WithPath(absPath);
  } catch (...) {
    failure = ImportError::Make(ImportError::Code::Internal,
                                "Importer threw unknown exception")
                  .WithPath(absPath);
  }

  lock.lock();

  if (AssetRecord *rec = MutableRecordLocked(handle)) {
    if (rec->settingsHash != settingsHash) {
      rec->settingsHash = settingsHash;
      m_dirty = true;
    }
  }

  m_inflight.erase(handle);

  if (failure) {
    m_cache.MarkFailed(handle, std::move(*failure));
  } else {
    m_cache.Insert(handle, asset, AssetLoadState::Loaded);
  }

  m_cv.notify_all();
  return asset;
}

std::shared_ptr<Asset> AssetManagerRuntime::EnsureGpu(AssetHandle handle) {
  auto asset = PreloadCpu(handle);
  if (!asset)
    return nullptr;

  if (asset->IsGpuReady())
    return asset;

  std::lock_guard<std::mutex> gpuLock(m_gpuMutex);

  if (asset->IsGpuReady())
    return asset;

  if (!asset->EnsureGpu()) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cache.MarkFailed(handle, ImportError::Make(ImportError::Code::Internal,
                                                 "EnsureGpu failed (see log)"));
    return nullptr;
  }

  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cache.MarkUploaded(handle);
  }

  return asset;
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

  if (record->builtin)
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

// std::shared_ptr<Asset>
// AssetManagerRuntime::PreloadCpuLocked(AssetHandle handle) {

//
// std::shared_ptr<Asset>
// AssetManagerRuntime::EnsureGpuLocked(AssetHandle handle) {
//   auto asset = PreloadCpuLocked(handle);
//   if (!asset)
//     return nullptr;
//
//   if (asset->IsGpuReady())
//     return asset;
//
//   if (!asset->EnsureGpu()) {
//     m_cache.MarkFailed(handle, ImportError::Make(ImportError::Code::Internal,
//                                                  "EnsureGpu failed (see
//                                                  log)"));
//     return nullptr;
//   }
//
//   m_cache.MarkUploaded(handle);
//   return asset;
// }

} // namespace engine::asset
