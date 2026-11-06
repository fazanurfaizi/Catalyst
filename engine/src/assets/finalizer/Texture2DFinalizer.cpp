#include "engine/asset/finalizer/Texture2DFinalizer.hpp"
#include "engine/asset/importer/texture2d/Texture2DImportResult.hpp"
#include "engine/graphics/assets/Texture2DAsset.hpp"
#include "engine/utils/Buffer.hpp"
#include "engine/utils/Logger.hpp"
#include <memory>

namespace engine::asset {

namespace {
graphics::ImageFormat ToImageFormat(TextureFormat format) {
  switch (format) {
  case TextureFormat::RGB8:
    return graphics::ImageFormat::RGB8;
  case TextureFormat::RGBA8:
    return graphics::ImageFormat::RGBA8;
  }
  return graphics::ImageFormat::RGBA8;
}
} // namespace

std::shared_ptr<Asset>
Texture2DFinalizer::Finalize(AssetHandle handle,
                             const Texture2DImportResult &data) {
  if (data.pixelBytes.empty() || data.width == 0 || data.height == 0) {
    Logger::Error("[Texture2DFinalizer] empty pixel data");
    return nullptr;
  }

  graphics::TextureSpecification spec;
  spec.width = static_cast<int>(data.width);
  spec.height = static_cast<int>(data.height);
  spec.format = ToImageFormat(data.format);

  Buffer buffer;
  buffer.data = const_cast<std::uint8_t *>(data.pixelBytes.data());
  buffer.size = data.pixelBytes.size();

  std::shared_ptr<Asset> texture =
      graphics::Texture2DAsset::Create(handle, spec, buffer);

  return texture;
}

} // namespace engine::asset
