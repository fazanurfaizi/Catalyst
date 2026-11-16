#include "engine/asset/finalizer/ShaderFinalizer.hpp"
#include "engine/asset/importer/shader/ShaderImportResult.hpp"
#include "engine/resources/ShaderAsset.hpp"
#include "engine/utils/Logger.hpp"

namespace engine::asset {

std::shared_ptr<Asset> ShaderFinalizer::Finalize(AssetHandle handle,
                                                 const ShaderImportResult &data,
                                                 bool eagerGpu) {
  if (data.vertexSource.empty() || data.fragmentSource.empty()) {
    Logger::Error("[ShaderFinalizer] '%s': empty source",
                  data.debugName.c_str());
    return nullptr;
  }

  auto asset = std::make_shared<resources::ShaderAsset>(handle);
  asset->cpu = std::make_shared<const ShaderImportResult>(data);

  if (eagerGpu) {
    if (!asset->EnsureGpu()) {
      Logger::Error("[ShaderFinalizer] '%s': GPU compile failed",
                    data.debugName.c_str());
      return nullptr;
    }
  }

  return std::static_pointer_cast<Asset>(std::move(asset));
}

} // namespace engine::asset
