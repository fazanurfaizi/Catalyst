#pragma once

#include "engine/assets/core/Asset.hpp"
#include "engine/assets/core/AssetHandle.hpp"

#include <memory>

namespace engine::asset {

class SoundImportResult;

class SoundFinalizer {
public:
  static std::shared_ptr<Asset> Finalize(AssetHandle handle,
                                         const SoundImportResult &data,
                                         [[maybe_unused]] bool eagerGpu = true);
};

} // namespace engine::asset
