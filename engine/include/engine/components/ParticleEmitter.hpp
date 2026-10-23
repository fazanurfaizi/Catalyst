#pragma once

#include "engine/components/RenderLayer.hpp"
#include <glm/glm.hpp>

namespace engine {

class Texture;

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

  glm::vec4 startColor{1.0f, 1.0f, 1.0f, 1.0f};
  glm::vec4 endColor{1.0f, 0.0f, 0.0f, 0.0f};

  const Texture *texture = nullptr;
  glm::vec2 uvMin{0.0f, 0.0f};
  glm::vec2 uvMax{1.0f, 1.0f};
  RenderLayer layer{RenderLayer::Overlay};
};

} // namespace engine
