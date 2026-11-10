#include "engine/asset/importer/texture2d/Texture2DImportSettings.hpp"

namespace engine::asset {

std::uint64_t Texture2DImportSettings::ComputeHash() const {
  auto h = detail::kHashSeed;
  detail::HashCombine(h, magFilterLinear);
  return h;
}

json Texture2DImportSettings::ToJson() const {
  return {
      {"magFilterLinear", magFilterLinear},
  };
}

void Texture2DImportSettings::FromJson(const json &j) {
  if (!j.is_object())
    return;
  if (j.contains("magFilterLinear") && j["magFilterLinear"].is_boolean()) {
    magFilterLinear = j["magFilterLinear"].get<bool>();
  }
}

} // namespace engine::asset
