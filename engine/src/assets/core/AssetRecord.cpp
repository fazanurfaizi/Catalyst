#include "engine/asset/core/AssetRecord.hpp"

namespace engine::asset {

const char *AssetRecordStateToString(AssetRecordState state) noexcept {
  switch (state) {
  case AssetRecordState::Valid:
    return "Valid";
  case AssetRecordState::Missing:
    return "Missing";
  case AssetRecordState::Broken:
    return "Broken";
  }
  return "Broken";
}

AssetRecordState AssetRecordStateFromString(const std::string &str) noexcept {
  if (str == "Valid")
    return AssetRecordState::Valid;
  if (str == "Missing")
    return AssetRecordState::Missing;
  if (str == "Broken")
    return AssetRecordState::Broken;
  return AssetRecordState::Broken;
}

} // namespace engine::asset
