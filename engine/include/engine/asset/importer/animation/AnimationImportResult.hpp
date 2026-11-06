#pragma once

#include "engine/asset/core/ImportedAssetData.hpp"
#include <glm/glm.hpp>
#include <string>
#include <variant>
#include <vector>

namespace engine::asset {

struct ParsedKeyframe {
  float time = 0.0f;
  std::variant<float, glm::vec2, glm::vec3, glm::vec4> value;
};

struct ParsedTrack {
  std::string propertyName;
  std::vector<ParsedKeyframe> keyframes;
};

class AnimationImportResult : public ImportedAssetData {
public:
  AssetType GetAssetType() const override { return AssetType::Animation; }

  std::string name;
  float duration = 0.0f;
  std::vector<ParsedTrack> tracks;
};
} // namespace engine::asset
