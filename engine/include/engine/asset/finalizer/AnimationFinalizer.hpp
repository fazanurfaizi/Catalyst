#pragma once

#include "engine/asset/core/Asset.hpp"
#include "engine/asset/core/AssetHandle.hpp"
#include <memory>

namespace engine::asset {

class AnimationImportResult;

class AnimationFinalizer {
public:
  static std::shared_ptr<Asset> Finalize(AssetHandle handle,
                                         const AnimationImportResult &data,
                                         [[maybe_unused]] bool eagerGpu = true);
};

} // namespace engine::asset
