#include "engine/asset/finalizer/FontFinalizer.hpp"
#include "engine/asset/core/Asset.hpp"
#include "engine/asset/importer/font/FontImportResult.hpp"
#include "engine/graphics/assets/FontAsset.hpp"
#include "engine/graphics/assets/Texture2DAsset.hpp"
#include "engine/graphics/text/Font.hpp"
#include "engine/utils/Buffer.hpp"
#include "engine/utils/Logger.hpp"
#include <memory>

namespace engine::asset {

std::shared_ptr<Asset> FontFinalizer::Finalize(AssetHandle handle,
                                               const FontImportResult &data) {
  if (data.atlasPixels.size() !=
      static_cast<std::size_t>(data.atlasWidth) *
          static_cast<std::size_t>(data.atlasHeight)) {
    Logger::Error("[FontFinalizer] atlas pixel buffer size mismatch");
    return nullptr;
  }

  // Expand R8 coverage to RGBA8 with white RGB + coverage alpha.
  std::vector<std::uint8_t> rgba(data.atlasPixels.size() * 4);
  for (std::size_t i = 0; i < data.atlasPixels.size(); ++i) {
    const std::uint8_t c = data.atlasPixels[i];
    rgba[i * 4 + 0] = 255;
    rgba[i * 4 + 1] = 255;
    rgba[i * 4 + 2] = 255;
    rgba[i * 4 + 3] = c;
  }

  graphics::TextureSpecification spec;
  spec.width = static_cast<int>(data.atlasWidth);
  spec.height = static_cast<int>(data.atlasHeight);
  spec.format = graphics::ImageFormat::RGBA8;
  spec.magFilterLinear = false;

  Buffer buffer;
  buffer.data = rgba.data();
  buffer.size = rgba.size();

  std::shared_ptr<graphics::Texture2DAsset> atlas =
      graphics::Texture2DAsset::Create(AssetHandle::Null(), spec, buffer);
  if (!atlas) {
    Logger::Error("[FontFinalizer] atlas texture creation failed");
    return nullptr;
  }

  std::shared_ptr<graphics::FontAsset> font =
      std::make_shared<graphics::FontAsset>(handle);
  font->atlasTexutre = std::move(atlas);
  font->textHeight = data.textHeight;
  font->glyphs.resize(data.glyphs.size());

  for (std::size_t i = 0; i < data.glyphs.size(); ++i) {
    const GlyphCpu &src = data.glyphs[i];
    graphics::Glyph &dst = font->glyphs[i];
    for (int k = 0; k < 4; ++k) {
      dst.uvMin = src.texCoords[k];
      dst.uvMax = src.positions[k];
    }
    dst.size = src.size;
    dst.advance = src.advance;
  }

  return std::static_pointer_cast<Asset>(std::move(font));
}

} // namespace engine::asset
