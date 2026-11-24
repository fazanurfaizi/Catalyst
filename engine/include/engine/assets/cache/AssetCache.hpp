#pragma once

#include "engine/assets/cache/AssetState.hpp"
#include "engine/assets/core/Asset.hpp"
#include "engine/assets/core/AssetHandle.hpp"
#include "engine/assets/core/ImportError.hpp"
#include <cstddef>
#include <map>
#include <memory>
#include <optional>

namespace engine::asset {

class AssetCache {
public:
  AssetCache() = default;
  ~AssetCache() = default;

  AssetCache(const AssetCache &) = delete;
  AssetCache &operator=(const AssetCache &) = delete;

  std::shared_ptr<Asset> FindAny(AssetHandle handle);

  /** Returns the loaded asset, or nullptr. Demotes expired entries to Unloaded
   */
  std::shared_ptr<Asset> FindUploaded(AssetHandle handle);

  /** Current runtime state. Returns Unloaded for unknown handles */
  AssetLoadState State(AssetHandle handle) const;

  /** Last ImportError, or nullptr if the state is not failed */
  const std::optional<ImportError> LastError(AssetHandle handle) const;

  /** Unloaded -> Loading */
  void MarkLoading(AssetHandle handle);

  /** Loading -> Loaded. Stores the asset in the cache. */
  void Insert(AssetHandle handle, std::shared_ptr<Asset> asset,
              AssetLoadState state = AssetLoadState::Loaded);

  // Uploaded to GPU
  void MarkUploaded(AssetHandle handle);

  /** Any -> Failed. Records the error, doesn't store an asset */
  void MarkFailed(AssetHandle handle, ImportError error);

  /** Removes the entry entirely. Next acquire starts fresh. */
  void Clear(AssetHandle handle);

  std::size_t GetTrackedCount() const { return m_entries.size(); }
  std::size_t GetLoadedCount() const;

private:
  struct Entry {
    std::shared_ptr<Asset> asset;
    AssetLoadState state = AssetLoadState::Unloaded;
    ImportError lastError;
  };

  std::map<AssetHandle, Entry> m_entries;
};

} // namespace engine::asset
