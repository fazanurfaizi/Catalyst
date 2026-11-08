#include "engine/asset/cache/AssetCache.hpp"

namespace engine::asset {

std::shared_ptr<Asset> AssetCache::FindLoaded(AssetHandle handle) {
  auto it = m_entries.find(handle);
  if (it == m_entries.end())
    return nullptr;

  if (it->second.state == AssetLoadState::Loaded) {
    if (auto locked = it->second.asset.lock()) {
      return locked;
    }
    // Weak expired
    it->second.state = AssetLoadState::Unloaded;
    it->second.asset.reset();
  }
  return nullptr;
}

AssetLoadState AssetCache::State(AssetHandle handle) const {
  auto it = m_entries.find(handle);
  if (it == m_entries.end())
    return AssetLoadState::Unloaded;

  if (it->second.state == AssetLoadState::Loaded &&
      it->second.asset.expired()) {
    return AssetLoadState::Unloaded;
  }
  return it->second.state;
}

const ImportError *AssetCache::LastError(AssetHandle handle) const {
  auto it = m_entries.find(handle);
  if (it == m_entries.end())
    return nullptr;
  if (it->second.state != AssetLoadState::Failed)
    return nullptr;
  return &it->second.lastError;
}

void AssetCache::MarkLoading(AssetHandle handle) {
  auto &entry = m_entries[handle];
  entry.state = AssetLoadState::Loading;
  entry.lastError = {};
  entry.asset.reset();
}

void AssetCache::Insert(AssetHandle handle, std::shared_ptr<Asset> asset) {
  auto &entry = m_entries[handle];
  entry.asset = asset;
  entry.state = AssetLoadState::Loaded;
  entry.lastError = {};
}

void AssetCache::MarkFailed(AssetHandle handle, ImportError error) {
  auto &entry = m_entries[handle];
  entry.state = AssetLoadState::Failed;
  entry.lastError = std::move(error);
  entry.asset.reset();
}

void AssetCache::Clear(AssetHandle handle) { m_entries.erase(handle); }

std::size_t AssetCache::GetLoadedCount() const {
  std::size_t count = 0;
  for (const auto &[handle, entry] : m_entries) {
    if (entry.state == AssetLoadState::Loaded && !entry.asset.expired()) {
      ++count;
    }
  }
  return count;
}

} // namespace engine::asset
