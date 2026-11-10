#pragma once

#include "RenderLayer.hpp"
#include "engine/asset/core/AssetHandle.hpp"
#include "engine/math/Color.hpp"
#include <glm/glm.hpp>
#include <string>

namespace engine::components {

struct ParticleEmitter {
  float emitRate{50.0f};
  float emitAccumulator{0.0f};

  float duration{2.0f};
  float timeEmitted{0.0f};

  float minLife{0.5f};
  float maxLife{1.5f};
  float minSpeed{1.0f};
  float maxSpeed{5.0f};
  float minScale{0.1f};
  float maxScale{0.3f};

  math::Color startColor{1.0f, 1.0f, 1.0f, 1.0f};
  math::Color endColor{1.0f, 0.0f, 0.0f, 0.0f};

  asset::AssetHandle atlas{};
  std::string regionName = "solidWhite";
  RenderLayer layer{RenderLayer::Overlay};
};

} // namespace engine::components
