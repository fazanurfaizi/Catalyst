#include "engine/asset/importer/font/FontImportSettings.hpp"
#include "engine/asset/importer/ImportSettings.hpp"

namespace engine::asset {

std::uint64_t FontImportSettings::ComputeHash() const {
  auto h = detail::kHashSeed;
  detail::HashCombine(h, pixelSize);
  detail::HashCombine(h, atlasWidth);
  detail::HashCombine(h, atlasHeight);
  detail::HashCombine(h, firstCodepoint);
  detail::HashCombine(h, charCount);
  return h;
}

json FontImportSettings::ToJson() const {
  // clang-format off
  return {
    {"pixelSize", pixelSize},
    {"atlasWidth", atlasWidth},
    {"atlasHeight", atlasHeight},
    {"firstCodepoint", firstCodepoint},
    {"charCount", charCount},
  };
  // clang-format on
}

void FontImportSettings::FromJson(const json &j) {
  if (!j.is_object())
    return;
  if (j.contains("pixelSize") && j["pixelSize"].is_number())
    pixelSize = j["pixelSize"].get<float>();
  if (j.contains("atlasWidth") && j["atlasWidth"].is_number())
    atlasWidth = j["atlasWidth"].get<float>();
  if (j.contains("atlasHeight") && j["atlasHeight"].is_number_integer())
    atlasHeight = j["atlasHeight"].get<int>();
  if (j.contains("firstCodepoint") && j["firstCodepoint"].is_number_unsigned())
    firstCodepoint = j["firstCodepoint"].get<std::uint32_t>();
  if (j.contains("charCount") && j["charCount"].is_number_unsigned())
    charCount = j["charCount"].get<std::uint32_t>();

  if (pixelSize <= 0.0f || pixelSize > 4096.0f)
    pixelSize = 180.0f;
  if (atlasWidth <= 0 || atlasWidth > 8192)
    atlasWidth = 1024;
  if (atlasHeight <= 0 || atlasHeight > 8192)
    atlasHeight = 1024;
  if (charCount == 0 || charCount > 4096)
    charCount = 95;
}

} // namespace engine::asset
