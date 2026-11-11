#include "engine/asset/finalizer/FontFinalizer.hpp"
#include "engine/asset/core/Asset.hpp"
#include "engine/asset/importer/font/FontImportResult.hpp"
#include "engine/graphics/text/Font.hpp"
#include "engine/resources/FontAsset.hpp"
#include "engine/utils/Logger.hpp"
#include <memory>

namespace engine::asset {

namespace {
graphics::Glyph Convert(const GlyphCpu &src) {
  graphics::Glyph g;

  const glm::vec2 &tl = src.texCoords[0];
  const glm::vec2 &br = src.texCoords[2];

  g.uvMin = {tl.x, br.y};
  g.uvMax = {br.x, tl.y};

  g.size = glm::ivec2(src.size);
  g.advance = src.advance;
  g.bearing = glm::ivec2(src.positions[0]);

  return g;
}
} // namespace

std::shared_ptr<Asset> FontFinalizer::Finalize(AssetHandle handle,
                                               const FontImportResult &data) {
  if (data.atlasPixels.size() !=
      static_cast<std::size_t>(data.atlasWidth) *
          static_cast<std::size_t>(data.atlasHeight)) {
    Logger::Error("[FontFinalizer] atlas pixel buffer size mismatch");
    return nullptr;
  }

  auto font = std::make_shared<graphics::Font>();
  if (!font->UploadAtlasFromR8(data.atlasWidth, data.atlasHeight,
                               data.atlasPixels.data())) {
    Logger::Error("[FontFinalizer] atlas upload failed");
    return nullptr;
  }

  for (std::size_t i = 0; i < data.glyphs.size(); ++i) {
    font->SetGlyph(data.firstCodepoint + static_cast<std::uint32_t>(i),
                   Convert(data.glyphs[i]));
  }

  font->SetMetrics(data.ascent, data.descent, data.lineGap);

  auto fontAsset = std::make_shared<resources::FontAsset>(handle);
  fontAsset->font = std::move(font);
  fontAsset->textHeight = data.textHeight;
  return std::static_pointer_cast<Asset>(std::move(fontAsset));
}

} // namespace engine::asset
