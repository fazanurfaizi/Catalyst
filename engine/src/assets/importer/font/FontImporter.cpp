#include "engine/asset/importer/font/FontImporter.hpp"
#include "engine/asset/importer/font/FontImportResult.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <ios>
#include <limits>
#include <stb/stb_truetype.h>
#include <vector>

namespace engine::asset {

namespace {

ImportResult<ImportedAssetDataPtr> ImportFont(const ImportRequest &request) {
  const FontImportSettings *settings = nullptr;
  if (request.settings &&
      request.settings->GetImporterId() == "catalyst.font") {
    settings = static_cast<const FontImportSettings *>(request.settings);
  }
  static const FontImportSettings kDefaultSettings{};
  if (!settings)
    settings = &kDefaultSettings;

  const float pixelSize = settings->pixelSize;
  const float atlasW = settings->atlasWidth;
  const float atlasH = settings->atlasHeight;
  const std::uint32_t firstCodepoint = settings->firstCodepoint;
  const std::uint32_t charCount = settings->charCount;

  if (!(pixelSize >= 1.0f && pixelSize <= 4096.0f)) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::MalformedContent,
                          "pixelSize must be in [1, 4096]")
            .WithPath(request.absolutePath));
  }

  if (atlasW < 256 || atlasW > 8192 || atlasH < 256 || atlasH > 8192) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::MalformedContent,
                          "atlas dimensions must be in [256, 8192]")
            .WithPath(request.absolutePath));
  }

  if (charCount == 0 || charCount > 4096) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::MalformedContent,
                          "charCount must be in [1, 4096]")
            .WithPath(request.absolutePath));
  }

  const std::uint64_t lastCodepoint =
      static_cast<std::uint64_t>(firstCodepoint) + charCount - 1;
  if (lastCodepoint > 0x10FFFFull) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::MalformedContent,
                          "firstCodepoint + charCount exceeds Unicode range")
            .WithPath(request.absolutePath));
  }

  const std::uint64_t pixelCount =
      static_cast<std::uint64_t>(atlasW) * static_cast<std::uint64_t>(atlasH);
  if (pixelCount > 64ull * 1024ull * 1024ull) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::TooLarge,
                          "font atlas exceeds 64 MiB of coverage")
            .WithPath(request.absolutePath));
  }

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

  if (static_cast<std::uint64_t>(fileSize) >
      static_cast<std::uint64_t>(std::numeric_limits<std::streamsize>::max())) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::TooLarge, "Font file too large")
            .WithPath(request.absolutePath));
  }

  // Allocate the buffer
  std::vector<uint8_t> fontDataBuf(static_cast<std::size_t>(fileSize));
  if (!in.read(reinterpret_cast<char *>(fontDataBuf.data()),
               static_cast<std::streamsize>(fileSize))) {
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
  const float pxPerUnit = stbtt_ScaleForPixelHeight(&fontInfo, pixelSize);
  const float ascent = static_cast<float>(rawAscent) * pxPerUnit;
  const float descent = static_cast<float>(rawDescent) * pxPerUnit; // negative
  const float lineGap = static_cast<float>(rawLineGap) * pxPerUnit;

  // Single-channel coverage
  std::vector<std::uint8_t> atlasPixels(atlasW * atlasH, 0);

  stbtt_pack_context ctx{};
  if (!stbtt_PackBegin(&ctx, atlasPixels.data(), atlasW, atlasH, 0, 1,
                       nullptr)) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::Internal, "stbtt_PackBegin failed")
            .WithPath(request.absolutePath));
  }

  std::vector<stbtt_packedchar> packedChars(charCount);
  const int packResult = stbtt_PackFontRange(
      &ctx, fontDataBuf.data(), 0, pixelSize, static_cast<int>(firstCodepoint),
      static_cast<int>(charCount), packedChars.data());

  stbtt_PackEnd(&ctx);

  if (!packResult) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::TooLarge,
                          "Font atlas packing failed - increase atlas size or "
                          "reduce font pixel size")
            .WithPath(request.absolutePath));
  }

  const float lineHeight = ascent - descent + lineGap;

  auto result = std::make_unique<FontImportResult>();
  result->atlasWidth = atlasW;
  result->atlasHeight = atlasH;
  result->atlasPixels = std::move(atlasPixels);
  result->firstCodepoint = firstCodepoint;
  result->textHeight =
      static_cast<std::uint32_t>(std::ceil(std::max(lineHeight, 1.0f)));
  result->ascent = ascent;
  result->descent = descent;
  result->lineGap = lineGap;
  result->glyphs.resize(charCount);

  for (std::uint32_t i = 0; i < charCount; ++i) {
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
