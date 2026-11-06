#pragma once

#include "engine/asset/core/Asset.hpp"
#include "engine/asset/core/AssetHandle.hpp"
#include <memory>

namespace engine::asset {

class SoundImportResult;

class SoundFinalizer {
public:
  static std::shared_ptr<Asset> Finalize(AssetHandle handle,
                                         const SoundImportResult &data);
};

} // namespace engine::asset
