#pragma once

#include "engine/asset/cache/AssetState.hpp"
#include "engine/asset/core/Asset.hpp"
#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/AssetRecord.hpp"
#include "engine/asset/core/ImportError.hpp"
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>

namespace engine::asset {

class IAssetManager {
public:
  virtual ~IAssetManager() = default;

  /** Load-or-return existing, type-erased.
   * Returns nullptr on failure.
   */
  virtual std::shared_ptr<Asset> AcquireRaw(AssetHandle handle) = 0;

  template <typename T> std::shared_ptr<T> Acquire(AssetHandle handle) {
    static_assert(std::is_base_of_v<Asset, T>,
                  "Acquire<T> requires T to derive from Asset");
    static_assert(!std::is_same_v<Asset, T>,
                  "Acquire<T> requires a concrete asset type, not Asset");

    auto asset = AcquireRaw(handle);
    if (!asset)
      return nullptr;

    if (asset->Type() != T::StaticType()) {
      return nullptr;
    }

    return std::static_pointer_cast<T>(std::move(asset));
  }

  virtual AssetHandle FindByName(std::string_view name) const = 0;

  template <typename T>
  std::shared_ptr<T> AcquireByName(std::string_view name) {
    static_assert(std::is_base_of_v<Asset, T>,
                  "AcquireByName<T> requires T to derive from Asset");
    static_assert(!std::is_same_v<Asset, T>,
                  "AcquireByName<T> requires a concrete asset type");

    const AssetHandle handle = FindByName(name);
    if (handle.IsNull())
      return nullptr;
    return Acquire<T>(handle);
  }

  virtual AssetLoadState State(AssetHandle handle) const = 0;

  /** Record for handle, or nullptr if unknown. */
  virtual const AssetRecord *Record(AssetHandle handle) const = 0;

  /** Last import error if state == Failed; nullptr otherwise. */
  virtual const ImportError *LastError(AssetHandle handle) const = 0;

  /** Clears the failed state and reloads. No-op if state != Failed. */
  virtual void Retry(AssetHandle handle) = 0;

  /** Drops the cache entry. The asset lives while external shared_ptr exist. */
  virtual void Release(AssetHandle handle) = 0;

  /** Asset type of the handle, or AssetType::None if unknown. */
  virtual AssetType Type(AssetHandle handle) const = 0;

  /** True if the handle refers to a known record. */
  virtual bool IsValid(AssetHandle handle) const = 0;

  virtual void AddRecord(AssetRecord record) = 0;

  /** Clears the cache entry and reloads. Unlike Retry, works from any
   *  state (Loaded included).
   *  */
  virtual void Reimport(AssetHandle handle) = 0;

  /** Increments on every successful or attempted Reimport. */
  virtual std::uint64_t GetReloadGeneration() const = 0;
};

} // namespace engine::asset
