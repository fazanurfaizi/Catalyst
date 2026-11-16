#pragma once

#include "engine/asset/core/Asset.hpp"
#include "engine/asset/core/AssetHandle.hpp"
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
