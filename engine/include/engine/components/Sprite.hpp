#pragma once

#include <glm/glm.hpp>
#include <string>

#include "RenderLayer.hpp"
#include "engine/asset/core/AssetHandle.hpp"
#include "engine/math/Color.hpp"

namespace engine::components {

struct Sprite {
  math::Color color = engine::math::kColorWhite;
  asset::AssetHandle atlas{};
  std::string regionName;
  RenderLayer layer{RenderLayer::Entities};
  float ySortKey = 0.0f;
};

} // namespace engine::components
