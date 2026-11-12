#include "engine/resources/FontAsset.hpp"
#include "engine/asset/importer/font/FontImportResult.hpp"
#include "engine/graphics/text/Font.hpp"
#include "engine/utils/Logger.hpp"

#include <cstddef>
#include <glm/glm.hpp>

namespace engine::resources {

namespace {

graphics::Glyph ConvertGlyph(const asset::GlyphCpu &src) {
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

bool FontAsset::EnsureGpu() const {
  if (m_gpu)
    return true;

  if (!cpu) {
    Logger::Error("[FontAsset] EnsureGpu: no CPU data");
    return false;
  }

  const std::size_t expected = static_cast<std::size_t>(cpu->atlasWidth) *
                               static_cast<std::size_t>(cpu->atlasHeight);

  if (cpu->atlasPixels.size() != expected) {
    Logger::Error("[FontAsset] atlas pixel buffer size mismatch: "
                  "have %zu, expected %zu",
                  cpu->atlasPixels.size(), expected);
    return false;
  }

  auto font = std::make_shared<graphics::Font>();
  if (!font->UploadAtlasFromR8(cpu->atlasWidth, cpu->atlasHeight,
                               cpu->atlasPixels.data())) {
    Logger::Error("[FontAsset] atlas upload failed");
    return false;
  }

  for (std::size_t i = 0; i < cpu->glyphs.size(); ++i) {
    font->SetGlyph(cpu->firstCodepoint + static_cast<std::uint32_t>(i),
                   ConvertGlyph(cpu->glyphs[i]));
  }

  font->SetMetrics(cpu->ascent, cpu->descent, cpu->lineGap);

  m_gpu = std::move(font);
  return true;
}

void FontAsset::ReleaseGpu() const { m_gpu.reset(); }

} // namespace engine::resources
