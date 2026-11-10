#include "engine/asset/finalizer/TextureAtlasFinalizer.hpp"
#include "engine/asset/importer/texture-atlas/TextureAtlasImportResult.hpp"
#include "engine/utils/Logger.hpp"
#include "glad/gl.h"
#include <memory>

namespace engine::asset {

std::shared_ptr<Asset>
TextureAtlasFinalizer::Finalize(AssetHandle handle,
                                const TextureAtlasImportResult &data) {
  if (data.rgba8.empty() || data.width == 0 || data.height == 0) {
    Logger::Error("[TextureAtlasFinalizer] empty atlas");
    return nullptr;
  }

  auto asset = std::make_shared<graphics::TextureAtlasAsset>(handle);

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

  // Reuse the same Texture::Upload path that Texture2DAsset uses.
  graphics::Texture texture;
  texture.Upload(static_cast<int>(data.width), static_cast<int>(data.height),
                 graphics::ImageFormat::RGBA8, data.rgba8.data(),
                 /*generateMipmaps=*/false);
  if (!texture.IsValid()) {
    Logger::Error("[TextureAtlasFinalizer] GL upload failed");
    return nullptr;
  }

  asset->AssignTexture(std::move(texture));
  for (const auto &[name, region] : data.regions) {
    asset->AssignRegion(name, region);
  }
  return std::static_pointer_cast<Asset>(std::move(asset));
}

} // namespace engine::asset
