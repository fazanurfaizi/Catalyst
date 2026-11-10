#include "engine/asset/importer/texture-atlas/TextureAtlasImportSettings.hpp"

namespace engine::asset {

std::uint64_t TextureAtlasImportSettings::ComputeHash() const {
  auto h = detail::kHashSeed;
  detail::HashCombine(h, maxAtlasDimension);
  detail::HashCombine(h, maxSpriteDimension);
  return h;
}

json TextureAtlasImportSettings::ToJson() const {
  return {
      {"maxAtlasDimension", maxAtlasDimension},
      {"maxSpriteDimension", maxSpriteDimension},
  };
}

void TextureAtlasImportSettings::FromJson(const json &j) {
  if (!j.is_object())
    return;
  if (j.contains("maxAtlasDimension") &&
      j["maxAtlasDimension"].is_number_integer()) {
    maxAtlasDimension = j["maxAtlasDimension"].get<int>();
  }
  if (j.contains("maxSpriteDimension") &&
      j["maxSpriteDimension"].is_number_integer()) {
    maxSpriteDimension = j["maxSpriteDimension"].get<int>();
  }

  if (maxAtlasDimension < 256 || maxAtlasDimension > 16384)
    maxAtlasDimension = 4096;
  if (maxSpriteDimension < 0 || maxSpriteDimension > 8192)
    maxSpriteDimension = 0;
}

} // namespace engine::asset
