#pragma once

#include "engine/asset/cache/AssetCache.hpp"
#include "engine/asset/core/Asset.hpp"
#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/AssetRecord.hpp"
#include "engine/asset/importer/ImporterRegistry.hpp"
#include "engine/asset/manager/IAssetManager.hpp"
#include <filesystem>
#include <map>
#include <memory>
#include <mutex>
#include <string_view>
#include <unordered_map>

namespace engine::asset {

class AssetManagerRuntime : public IAssetManager {
public:
  AssetManagerRuntime(ImporterRegistry &registry,
                      std::filesystem::path projectRoot);
  ~AssetManagerRuntime() = default;

  std::shared_ptr<Asset> AcquireRaw(AssetHandle handle) override;

  AssetLoadState State(AssetHandle handle) const override;

  AssetHandle FindByName(std::string_view name) const override;

  const AssetRecord *Record(AssetHandle handle) const override;

  const ImportError *LastError(AssetHandle handle) const override;

  void Retry(AssetHandle handle) override;

  void Release(AssetHandle handle) override;

  AssetType Type(AssetHandle handle) const override;

  bool IsValid(AssetHandle handle) const override;

  void AddRecord(AssetRecord record) override;

  void Reimport(AssetHandle handle) override;

  std::uint64_t GetReloadGeneration() const override;

  std::shared_ptr<Asset> PreloadCpu(AssetHandle handle) override;

  std::shared_ptr<Asset> EnsureGpu(AssetHandle handle) override;

protected:
  ImporterRegistry &m_registry;
  std::filesystem::path m_projectRoot;

  AssetCache m_cache;

  std::map<AssetHandle, AssetRecord> m_records;
  std::unordered_map<std::string, AssetHandle> m_nameToHandle;

  std::uint64_t m_reloadGeneration = 0;

private:
  const AssetRecord *RecordLocked(AssetHandle handle) const;
  AssetRecord *MutableRecordLocked(AssetHandle handle);

  std::shared_ptr<Asset> PreloadCpuLocked(AssetHandle handle);
  std::shared_ptr<Asset> EnsureGpuLocked(AssetHandle handle);

  mutable std::mutex m_mutex;
};

} // namespace engine::asset
