#include "engine/asset/importer/texture-atlas/TextureAtlasImporter.hpp"
#include "engine/asset/core/ImportError.hpp"
#include "engine/asset/importer/texture-atlas/TextureAtlasBuilder.hpp"
#include "engine/asset/importer/texture-atlas/TextureAtlasImportResult.hpp"
#include "engine/asset/importer/texture-atlas/TextureAtlasImportSettings.hpp"

#include <charconv>
#include <nlohmann/json.hpp>
#include <stb/stb_image.h>

#include <filesystem>
#include <fstream>

namespace engine::asset {

using nlohmann::json;

namespace {
std::optional<std::uint32_t> ParseHex(std::string_view hexStr) {
  if (!hexStr.empty() && hexStr[0] == '#') {
    hexStr.remove_prefix(1);
  } else if (hexStr.size() >= 2 && hexStr[0] == '0' &&
             (hexStr[1] == 'x' || hexStr[1] == 'X')) {
    hexStr.remove_prefix(2);
  }

  if (hexStr.empty())
    return std::nullopt;

  std::uint32_t result = 0;
  auto [ptr, ec] =
      std::from_chars(hexStr.data(), hexStr.data() + hexStr.size(), result, 16);

  if (ec != std::errc() || ptr != hexStr.data() + hexStr.size()) {
    return std::nullopt;
  }

  return result;
}

ImportResult<ImportedAssetDataPtr>
ImportTextureAtlas(const ImportRequest &request) {
  const TextureAtlasImportSettings *settings = nullptr;
  if (request.settings &&
      request.settings->GetImporterId() == "catalyst.texture_atlas") {
    settings =
        static_cast<const TextureAtlasImportSettings *>(request.settings);
  }
  static const TextureAtlasImportSettings kDefaultSettings{};
  if (!settings)
    settings = &kDefaultSettings;

  const int maxAtlasDimension = settings->maxAtlasDimension;
  const int maxSpriteDimension = settings->maxSpriteDimension;

  std::ifstream in(request.absolutePath);
  if (!in) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::FileUnreadable,
                          "Atlas manifest not readable")
            .WithPath(request.absolutePath));
  }

  json manifest;
  try {
    in >> manifest;
  } catch (const json::exception &e) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::MalformedContent, e.what())
            .WithPath(request.absolutePath));
  }

  TextureAtlasBuilder builder;

  const auto manifestDir = request.absolutePath.parent_path();

  if (!manifest.contains("sources") || !manifest["sources"].is_array()) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::MalformedContent,
                          "Manifest missing 'sources' array")
            .WithPath(request.absolutePath));
  }

  for (const json &entry : manifest["sources"]) {
    const std::string type = entry.value("type", "");

    if (type == "directory") {
      const auto dir = manifestDir / entry.value("path", "");
      builder.AddFromDirectory(dir.string(), maxSpriteDimension);
    } else if (type == "solid") {
      const std::string name = entry.value("name", "");
      if (entry.value("ifMissing", false) && builder.HasSource(name))
        continue;
      const auto color = ParseHex(entry.value("color", "FFFFFFFF"));
      if (!color) {
        return ImportResult<ImportedAssetDataPtr>::Err(
            ImportError::Make(ImportError::Code::MalformedContent,
                              "Invalid hex color in 'color'")
                .WithPath(request.absolutePath));
      }
      builder.AddSolid(name, *color, entry.value("size", 16));
    } else if (type == "checkerboard") {
      const std::string name = entry.value("name", "");
      const int cells = entry.value("cells", 1);
      if (entry.value("ifMissing", false) && builder.HasSource(name))
        continue;
      const auto colorA = ParseHex(entry.value("colorA", "FFFFFFFF"));
      const auto colorB = ParseHex(entry.value("colorB", "FFFFFFFF"));
      if (!colorA || !colorB) {
        return ImportResult<ImportedAssetDataPtr>::Err(
            ImportError::Make(ImportError::Code::MalformedContent,
                              "Invalid hex color in 'colorA' or 'colorB'")
                .WithPath(request.absolutePath));
      }
      builder.AddCheckerboard(name, cells, *colorA, *colorB);
    }
  }

  TextureAtlasBuilder::Packed packed;
  if (!builder.Pack(packed, maxAtlasDimension)) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::TooLarge, "Atlas packing failed")
            .WithPath(request.absolutePath));
  }

  auto result = std::make_unique<TextureAtlasImportResult>();
  result->width = static_cast<std::uint32_t>(packed.width);
  result->height = static_cast<std::uint32_t>(packed.height);
  result->rgba8 = std::move(packed.rgba8);
  result->regions = std::move(packed.regions);

  return ImportResult<ImportedAssetDataPtr>::Ok(std::move(result));
}

} // namespace

ImportResult<ImportedAssetDataPtr>
TextureAtlasImporter::Import(const ImportRequest &request) const {
  return ImportTextureAtlas(request);
}

} // namespace engine::asset
