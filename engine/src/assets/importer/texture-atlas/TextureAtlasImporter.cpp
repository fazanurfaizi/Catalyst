#include "engine/asset/importer/texture-atlas/TextureAtlasImporter.hpp"
#include "engine/asset/core/ImportError.hpp"
#include "engine/asset/importer/texture-atlas/TextureAtlasBuilder.hpp"
#include "engine/asset/importer/texture-atlas/TextureAtlasImportResult.hpp"
#include "engine/asset/importer/texture-atlas/TextureAtlasImportSettings.hpp"

#include <nlohmann/json.hpp>
#include <stb/stb_image.h>

#include <charconv>
#include <filesystem>
#include <fstream>
#include <optional>
#include <system_error>

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

std::optional<std::filesystem::path>
ResolveInside(const std::filesystem::path &base, const std::string &relative) {
  std::error_code ec;
  const auto canonicalBase = std::filesystem::weakly_canonical(base, ec);
  if (ec)
    return std::nullopt;

  const auto candidate = std::filesystem::weakly_canonical(base / relative, ec);
  if (ec)
    return std::nullopt;

  const auto rel = candidate.lexically_relative(canonicalBase);
  if (rel.empty())
    return std::nullopt;

  auto it = rel.begin();
  if (it != rel.end() && *it == "..")
    return std::nullopt;

  return candidate;
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

  for (std::size_t i = 0; i < manifest["sources"].size(); ++i) {
    const json &entry = manifest["sources"][i];

    if (!entry.is_object()) {
      return ImportResult<ImportedAssetDataPtr>::Err(
          ImportError::Make(ImportError::Code::MalformedContent,
                            "sources[" + std::to_string(i) +
                                "] is not an object")
              .WithPath(request.absolutePath));
    }

    if (!entry.contains("type") || !entry["type"].is_string()) {
      return ImportResult<ImportedAssetDataPtr>::Err(
          ImportError::Make(ImportError::Code::MalformedContent,
                            "sources[" + std::to_string(i) +
                                "] missing string 'type'")
              .WithPath(request.absolutePath));
    }

    const std::string type = entry.value("type", "");

    if (type == "directory") {
      if (!entry.contains("path") || !entry["path"].is_string()) {
        return ImportResult<ImportedAssetDataPtr>::Err(
            ImportError::Make(ImportError::Code::MalformedContent,
                              "directory source missing string 'path'")
                .WithPath(request.absolutePath));
      }

      const auto dir =
          ResolveInside(manifestDir, entry["path"].get<std::string>());
      if (!dir) {
        return ImportResult<ImportedAssetDataPtr>::Err(
            ImportError::Make(ImportError::Code::MalformedContent,
                              "directory source escapes the manifest root")
                .WithPath(request.absolutePath));
      }

      builder.AddFromDirectory(dir->string(), maxSpriteDimension);
    } else if (type == "solid") {
      const std::string name = entry.value("name", "");
      if (name.empty()) {
        return ImportResult<ImportedAssetDataPtr>::Err(
            ImportError::Make(ImportError::Code::MalformedContent,
                              "solid source requires a name")
                .WithPath(request.absolutePath));
      }
      if (entry.value("ifMissing", false) && builder.HasSource(name))
        continue;

      const auto color = ParseHex(entry.value("color", "FFFFFFFF"));
      if (!color) {
        return ImportResult<ImportedAssetDataPtr>::Err(
            ImportError::Make(ImportError::Code::MalformedContent,
                              "Invalid hex color in 'color'")
                .WithPath(request.absolutePath));
      }

      const int size = entry.value("size", 16);
      if (size < 1 || size > 4096) {
        return ImportResult<ImportedAssetDataPtr>::Err(
            ImportError::Make(ImportError::Code::MalformedContent,
                              "solid 'size' must be in [1, 4096]")
                .WithPath(request.absolutePath));
      }

      builder.AddSolid(name, *color, size);
    } else if (type == "checkerboard") {
      const std::string name = entry.value("name", "");
      if (name.empty()) {
        return ImportResult<ImportedAssetDataPtr>::Err(
            ImportError::Make(ImportError::Code::MalformedContent,
                              "checkerboard source requires a name")
                .WithPath(request.absolutePath));
      }
      if (entry.value("ifMissing", false) && builder.HasSource(name))
        continue;

      const int cells = entry.value("cells", 1);
      if (cells < 1 || cells > 512) {
        return ImportResult<ImportedAssetDataPtr>::Err(
            ImportError::Make(ImportError::Code::MalformedContent,
                              "checkerboard 'cells' must be in [1, 512]")
                .WithPath(request.absolutePath));
      }

      const auto colorA = ParseHex(entry.value("colorA", "FFFFFFFF"));
      const auto colorB = ParseHex(entry.value("colorB", "FFFFFFFF"));
      if (!colorA || !colorB) {
        return ImportResult<ImportedAssetDataPtr>::Err(
            ImportError::Make(ImportError::Code::MalformedContent,
                              "Invalid hex color in 'colorA' or 'colorB'")
                .WithPath(request.absolutePath));
      }
      builder.AddCheckerboard(name, cells, *colorA, *colorB);
    } else {
      return ImportResult<ImportedAssetDataPtr>::Err(
          ImportError::Make(ImportError::Code::MalformedContent,
                            "Unknown source type '" + type + "'")
              .WithPath(request.absolutePath));
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
