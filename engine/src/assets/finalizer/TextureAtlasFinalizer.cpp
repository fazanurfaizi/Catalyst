#include "engine/asset/finalizer/TextureAtlasFinalizer.hpp"
#include "engine/asset/importer/texture-atlas/TextureAtlasImportResult.hpp"
#include "engine/resources/TextureAtlasAsset.hpp"
#include "engine/resources/TextureFormat.hpp"
#include "engine/utils/Logger.hpp"
#include "glad/gl.h"
#include <memory>

namespace engine::asset {

std::shared_ptr<Asset> TextureAtlasFinalizer::Finalize(
    AssetHandle handle, const TextureAtlasImportResult &data, bool eagerGpu) {
  if (data.rgba8.empty() || data.width == 0 || data.height == 0) {
    Logger::Error("[TextureAtlasFinalizer] empty atlas");
    return nullptr;
  }

  GLint glMax = 0;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glMax);
  if (glMax <= 0) {
    Logger::Error(
        "[TextureAtlasFinalizer] no GL context (glGetIntegerv failed)");
    return nullptr;
  }
  if (static_cast<GLint>(data.width) > glMax ||
      static_cast<GLint>(data.height) > glMax) {
    Logger::Error("[TextureAtlasFinalizer] packed atlas %ux%u exceeds "
                  "GL_MAX_TEXTURE_SIZE %d on this device",
                  data.width, data.height, glMax);
    return nullptr;
  }

  resources::TextureSpecification spec;
  spec.width = static_cast<std::uint32_t>(data.width);
  spec.height = static_cast<std::uint32_t>(data.height);
  spec.format = resources::ImageFormat::RGBA8;
  spec.magFilterLinear = false;

  auto asset = std::make_shared<resources::TextureAtlasAsset>(handle);
  asset->AssignCpuPixels(spec, data.rgba8);

  for (const auto &[name, region] : data.regions) {
    asset->AssignRegion(name, region);
  }

  if (eagerGpu) {
    if (!asset->EnsureGpu()) {
      Logger::Error("[TextureAtlasFinalizer] GPU upload failed");
      return nullptr;
    }
  }

  return std::static_pointer_cast<Asset>(std::move(asset));
}

} // namespace engine::asset
