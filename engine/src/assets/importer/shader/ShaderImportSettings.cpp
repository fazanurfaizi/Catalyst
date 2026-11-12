#include "engine/asset/importer/shader/ShaderImportSettings.hpp"
#include "engine/asset/importer/ImportSettings.hpp"

namespace engine::asset {

std::uint64_t ShaderImportSettings::ComputeHash() const {
  auto h = detail::kHashSeed;
  return h;
}

json ShaderImportSettings::ToJson() const { return {}; }

void ShaderImportSettings::FromJson(const json &j) {
  if (!j.is_object())
    return;
}

} // namespace engine::asset
