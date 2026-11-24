#pragma once

#include "engine/assets/core/Asset.hpp"
#include "engine/assets/core/AssetHandle.hpp"

#include <memory>

namespace engine::asset {

class FontImportResult;

class FontFinalizer {
public:
  static std::shared_ptr<Asset> Finalize(AssetHandle handle,
                                         const FontImportResult &data,
                                         bool eagerGpu = true);
};

} // namespace engine::asset
