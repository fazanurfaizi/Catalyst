#include "engine/asset/finalizer/Texture2DFinalizer.hpp"
#include "engine/asset/importer/texture2d/Texture2DImportResult.hpp"
#include "engine/resources/Texture2DAsset.hpp"
#include "engine/utils/Buffer.hpp"
#include "engine/utils/Logger.hpp"
#include <memory>

namespace engine::asset {

namespace {
resources::ImageFormat ToImageFormat(TextureFormat format) {
  switch (format) {
  case TextureFormat::RGB8:
    return resources::ImageFormat::RGB8;
  case TextureFormat::RGBA8:
    return resources::ImageFormat::RGBA8;
  }
  return resources::ImageFormat::RGBA8;
}
} // namespace

std::shared_ptr<Asset>
Texture2DFinalizer::Finalize(AssetHandle handle,
                             const Texture2DImportResult &data) {
  if (data.pixelBytes.empty() || data.width == 0 || data.height == 0) {
    Logger::Error("[Texture2DFinalizer] empty pixel data");
    return nullptr;
  }

  resources::TextureSpecification spec;
  spec.width = static_cast<int>(data.width);
  spec.height = static_cast<int>(data.height);
  spec.format = ToImageFormat(data.format);
  spec.magFilterLinear = data.magFilterLinear;

  Buffer buffer;
  buffer.data = const_cast<std::uint8_t *>(data.pixelBytes.data());
  buffer.size = data.pixelBytes.size();

  std::shared_ptr<Asset> asset = resources::Texture2DAsset::Create(
      handle, spec, buffer, /*uploadImmediately=*/true);

  return std::static_pointer_cast<Asset>(std::move(asset));
}

} // namespace engine::asset
