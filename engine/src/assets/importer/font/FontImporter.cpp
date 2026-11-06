#include "engine/asset/importer/font/FontImporter.hpp"
#include "engine/asset/importer/font/FontImportResult.hpp"

#include <fstream>
#include <ios>
#include <stb/stb_truetype.h>
#include <vector>

namespace engine::asset {

namespace {

constexpr std::uint32_t kFirstCodepoint = 32; // ' ' (space)
constexpr std::uint32_t kCharCount = 95;      // printable ASCII
constexpr std::uint32_t kAtlasWidth = 1024;
constexpr std::uint32_t kAtlasHeight = 1024;
constexpr float kFontPixelSize = 180.0f;

ImportResult<ImportedAssetDataPtr> ImportFont(const ImportRequest &request) {
  std::ifstream in(request.absolutePath, std::ios::binary);
  if (!in) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::FileUnreadable,
                          "Could not open font file")
            .WithPath(request.absolutePath));
  }
  in.seekg(0, std::ios::end);
  const std::streamoff fileSize = in.tellg();
  in.seekg(0, std::ios::beg);
  if (fileSize <= 0) {
    return ImportResult<ImportedAssetDataPtr>::Err(ImportError::Make(
        ImportError::Code::FileUnreadable, "Font file is empty"));
  }

  // Allocate the buffer
  std::vector<uint8_t> fontDataBuf(static_cast<std::size_t>(fileSize));
  if (!in.read(reinterpret_cast<char *>(fontDataBuf.data()), fileSize)) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::FileUnreadable,
                          "Font file read failed")
            .WithPath(request.absolutePath));
  }

  stbtt_fontinfo fontInfo{};
  if (!stbtt_InitFont(&fontInfo, fontDataBuf.data(), 0)) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::MalformedContent,
                          "stbtt_InitFont failed - not a valid TTF")
            .WithPath(request.absolutePath));
  }

  int rawAscent = 0, rawDescent = 0, rawLineGap = 0;
  stbtt_GetFontVMetrics(&fontInfo, &rawAscent, &rawDescent, &rawLineGap);
  const float pxPerUnit = stbtt_ScaleForPixelHeight(&fontInfo, kFontPixelSize);
  const float ascent = static_cast<float>(rawAscent) * pxPerUnit;
  const float descent = static_cast<float>(rawDescent) * pxPerUnit; // negative
  const float lineGap = static_cast<float>(rawLineGap) * pxPerUnit;

  // Single-channel coverage
  std::vector<std::uint8_t> atlasPixels(kAtlasWidth * kAtlasHeight, 0);

  stbtt_pack_context ctx{};
  if (!stbtt_PackBegin(&ctx, atlasPixels.data(), static_cast<int>(kAtlasWidth),
                       static_cast<int>(kAtlasHeight), 0, 1, nullptr)) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::Internal, "stbtt_PackBegin failed")
            .WithPath(request.absolutePath));
  }

  stbtt_packedchar packedChars[kCharCount] = {};
  const int packResult =
      stbtt_PackFontRange(&ctx, fontDataBuf.data(), 0, kFontPixelSize,
                          static_cast<int>(kFirstCodepoint),
                          static_cast<int>(kCharCount), packedChars);

  stbtt_PackEnd(&ctx);

  if (!packResult) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::TooLarge,
                          "Font atlas packing failed - increase atlas size or "
                          "reduce font pixel size")
            .WithPath(request.absolutePath));
  }

  auto result = std::make_unique<FontImportResult>();
  result->atlasWidth = kAtlasWidth;
  result->atlasHeight = kAtlasHeight;
  result->atlasPixels = std::move(atlasPixels);
  result->firstCodepoint = kFirstCodepoint;
  result->textHeight = static_cast<std::uint32_t>(kFontPixelSize);
  result->ascent = ascent;
  result->descent = descent;
  result->lineGap = lineGap;
  result->glyphs.resize(kCharCount);

  const float atlasW = static_cast<float>(kAtlasWidth);
  const float atlasH = static_cast<float>(kAtlasHeight);

  for (std::uint32_t i = 0; i < kCharCount; ++i) {
    const stbtt_packedchar &pc = packedChars[i];
    GlyphCpu &g = result->glyphs[i];

    g.texCoords[0] = {pc.x0 / atlasW, pc.y0 / atlasH};
    g.texCoords[1] = {pc.x1 / atlasW, pc.y0 / atlasH};
    g.texCoords[2] = {pc.x1 / atlasW, pc.y1 / atlasH};
    g.texCoords[3] = {pc.x0 / atlasW, pc.y1 / atlasH};

    g.size = {pc.x1 - pc.x0, pc.y1 - pc.y0};
    g.advance = pc.xadvance;

    g.positions[0] = {pc.xoff, pc.yoff};
    g.positions[1] = {pc.xoff + g.size.x, pc.yoff};
    g.positions[2] = {pc.xoff + g.size.x, pc.yoff + g.size.y};
    g.positions[3] = {pc.xoff, pc.yoff + g.size.y};
  }

  return ImportResult<ImportedAssetDataPtr>::Ok(std::move(result));
}

} // namespace

ImportResult<ImportedAssetDataPtr>
FontImporter::Import(const ImportRequest &request) const {
  return ImportFont(request);
}

} // namespace engine::asset
