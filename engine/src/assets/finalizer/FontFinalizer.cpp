#include "engine/asset/finalizer/FontFinalizer.hpp"
#include "engine/asset/core/Asset.hpp"
#include "engine/asset/importer/font/FontImportResult.hpp"
#include "engine/resources/FontAsset.hpp"
#include "engine/utils/Logger.hpp"
#include <memory>

namespace engine::asset {

std::shared_ptr<Asset> FontFinalizer::Finalize(AssetHandle handle,
                                               const FontImportResult &data,
                                               bool eagerGpu) {
  const std::size_t expected = static_cast<std::size_t>(data.atlasWidth) *
                               static_cast<std::size_t>(data.atlasHeight);

  if (data.atlasPixels.size() != expected) {
    Logger::Error("[FontFinalizer] atlas pixel buffer size mismatch");
    return nullptr;
  }

  auto asset = std::make_shared<resources::FontAsset>(handle);
  asset->cpu = std::make_shared<const FontImportResult>(data);

  if (eagerGpu) {
    if (!asset->EnsureGpu()) {
      Logger::Error("[FontFinalizer] GPU upload failed for handle %s",
                    handle.ToString().c_str());
      return nullptr;
    }
  }

  return std::static_pointer_cast<Asset>(std::move(asset));
}

} // namespace engine::asset
