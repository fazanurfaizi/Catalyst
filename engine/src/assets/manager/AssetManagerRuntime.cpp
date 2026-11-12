#include "engine/asset/manager/AssetManagerRuntime.hpp"
#include "engine/asset/finalizer/AssetFinalizer.hpp"
#include "engine/asset/persistence/SidecarSerializer.hpp"
#include "engine/utils/Logger.hpp"
#include <filesystem>
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
    return;
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
  return EnsureGpu(handle);
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
  auto result = AcquireRaw(handle);
  if (result) {
    ++m_reloadGeneration;
  } else {
    Logger::Warn("[Asset] reimport failed for %s, generation not bumped",
                 handle.ToString().c_str());
  }
}

std::shared_ptr<Asset> AssetManagerRuntime::PreloadCpu(AssetHandle handle) {
  if (auto cached = m_cache.FindAny(handle)) {
    return cached;
  }

  const AssetRecord *record = Record(handle);
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

std::shared_ptr<Asset> AssetManagerRuntime::EnsureGpu(AssetHandle handle) {
  auto asset = PreloadCpu(handle);
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
