#include "engine/asset/importer/texture2d/Texture2DImporter.hpp"
#include "engine/asset/importer/texture2d/Texture2DImportResult.hpp"
#include "engine/asset/importer/texture2d/Texture2DImportSettings.hpp"

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <limits>
#include <stb/stb_image.h>

namespace engine::asset {

namespace {
ImportResult<ImportedAssetDataPtr>
ImportTexture2D(const ImportRequest &request) {
  const Texture2DImportSettings *settings = nullptr;
  if (request.settings &&
      request.settings->GetImporterId() == "catalyst.texture2d") {
    settings = static_cast<const Texture2DImportSettings *>(request.settings);
  }
  static const Texture2DImportSettings kDefaultSettings{};
  if (!settings)
    settings = &kDefaultSettings;

  std::error_code ec;
  if (!std::filesystem::is_regular_file(request.absolutePath, ec) || ec) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::FileNotFound,
                          "Texture file not found or not a regular file")
            .WithPath(request.absolutePath));
  }

  int w = 0, h = 0, channels = 0;
  std::string path_str = request.absolutePath.string();

  stbi_uc *raw = stbi_load(path_str.c_str(), &w, &h, &channels, 4);
  if (!raw) {
    const char *reason = stbi_failure_reason();
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::DecodeFailed,
                          reason ? reason : "Unknown stbi decode failure")
            .WithPath(request.absolutePath));
  }

  if (w <= 0 || h <= 0) {
    stbi_image_free(raw);
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::DecodeFailed,
                          "Decoded image has zero dimensions")
            .WithPath(request.absolutePath));
  }

  const std::uint64_t pixels =
      static_cast<std::uint64_t>(w) * static_cast<std::uint64_t>(h);
  if (pixels > std::numeric_limits<std::size_t>::max() / 4ull) {
    stbi_image_free(raw);
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::TooLarge,
                          "Image dimensions overflow pixel buffer size")
            .WithPath(request.absolutePath));
  }

  const std::size_t row_bytes = static_cast<std::size_t>(w) * 4ull;
  const std::size_t total = row_bytes * static_cast<std::size_t>(h);

  auto result = std::make_unique<Texture2DImportResult>();
  result->width = static_cast<std::uint32_t>(w);
  result->height = static_cast<std::uint32_t>(h);
  result->format = TextureFormat::RGBA8;
  result->pixelBytes.assign(raw, raw + total);
  result->magFilterLinear = settings->magFilterLinear;
  stbi_image_free(raw);

  // Flip rows to engine convention (bottom-up data)
  for (std::uint32_t y = 0; y < static_cast<std::uint32_t>(h) / 2; ++y) {
    std::uint8_t *top = result->pixelBytes.data() + y * row_bytes;
    std::uint8_t *bot = result->pixelBytes.data() +
                        (static_cast<std::size_t>(h) - 1 - y) * row_bytes;
    std::swap_ranges(top, top + row_bytes, bot);
  }

  return ImportResult<ImportedAssetDataPtr>::Ok(std::move(result));
}

} // namespace

ImportResult<ImportedAssetDataPtr>
Texture2DImporter::Import(const ImportRequest &request) const {
  return ImportTexture2D(request);
}

} // namespace engine::asset
