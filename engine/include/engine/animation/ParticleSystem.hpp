#pragma once

namespace engine {

class Scene;

class ParticleSystem {
public:
  static void Update(Scene &scene, float deltaTime);
};

} // namespace engine
