#pragma once

#include "engine/asset/cache/AssetState.hpp"
#include "engine/asset/core/Asset.hpp"
#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/ImportError.hpp"
#include <cstddef>
#include <map>
#include <memory>

namespace engine::asset {

class AssetCache {
public:
  AssetCache() = default;
  ~AssetCache() = default;

  AssetCache(const AssetCache &) = default;
  AssetCache &operator=(const AssetCache &) = default;

  /** Returns the loaded asset, or nullptr. Demotes expired entries to Unloaded
   */
  std::shared_ptr<Asset> FindLoaded(AssetHandle handle);

  /** Current runtime state. Returns Unloaded for unknown handles */
  AssetLoadState State(AssetHandle handle) const;

  /** Last ImportError, or nullptr if the state is not failed */
  const ImportError *LastError(AssetHandle handle) const;

  /** Unloaded -> Loading */
  void MarkLoading(AssetHandle handle);

  /** Loading -> Loaded. Stores the asset's week ptr */
  void Insert(AssetHandle handle, std::shared_ptr<Asset> asset);

  /** Any -> Failed. Records the error, doesn't store an asset */
  void MarkFailed(AssetHandle handle, ImportError error);

  /** Removes the entry entirely. Next acquire starts fresh. */
  void Clear(AssetHandle handle);

  std::size_t GetTrackedCount() const { return m_entries.size(); }
  std::size_t GetLoadedCount() const;

private:
  struct Entry {
    std::weak_ptr<Asset> asset;
    AssetLoadState state = AssetLoadState::Unloaded;
    ImportError lastError;
  };

  std::map<AssetHandle, Entry> m_entries;
};

} // namespace engine::asset
