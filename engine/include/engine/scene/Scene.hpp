#pragma once

#include "engine/animation/Animator.hpp"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace engine {
class Texture;
}

namespace engine {

using Entity = std::uint32_t;
constexpr Entity kInvalidEntity = 0xFFFFFFFFu;

enum class RenderLayer : std::uint8_t {
  Background = 0, // grid, sky, parallax
  Ground = 1,     // floor tiles, decor
  Shadows = 2,    // drop shadows
  Entities = 3,   // heroes, enemies, towers (default)
  Overlay = 4,    // effects, particles on top
  UI = 5,         // HUD, health bars
  COUNT,
};

struct Transform {
  glm::vec2 position{0.0f, 0.0f};
  glm::vec2 scale{1.0f, 1.0f};
  float rotation = 0.0f;
};

struct Sprite {
  glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
  const Texture *texture = nullptr;
  glm::vec2 uvMin{0.0f, 0.0f};
  glm::vec2 uvMax{1.0f, 1.0f};
  RenderLayer layer{RenderLayer::Entities};
  float ySortKey = 0.0f;
  std::string regionName{};
};

struct Particle {
  glm::vec2 velocity{0.0f};
  float life{1.0f};
  float maxLife{1.0f};
  float startScale{1.0f};
  float endScale{1.0f};
  glm::vec4 startColor{1.0f};
  glm::vec4 endColor{1.0f};
};

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

class Scene {
public:
  Entity CreateEntity(const std::string &name = "Entity");
  void DestroyEntity(Entity entity);
  bool IsAlive(Entity entity) const;
  std::size_t Size() const { return m_LiveCount; }

  Transform &TransformOf(Entity entity);
  const Transform &TransformOf(Entity entity) const;

  Sprite &SpriteOf(Entity entity);
  const Sprite &SpriteOf(Entity entity) const;

  const std::string &NameOf(Entity entity) const;

  Animator &AnimatorOf(Entity entity);
  const Animator &AnimatorOf(Entity entity) const;

  std::optional<Particle> &ParticleOf(Entity entity);
  const std::optional<Particle> &ParticleOf(Entity entity) const;

  std::optional<ParticleEmitter> &EmitterOf(Entity entity);
  const std::optional<ParticleEmitter> &EmitterOf(Entity entity) const;

  // fn(Entity, const Transform&, const Sprite&)
  template <typename Fn> void ForEach(Fn &&fn) const {
    for (std::size_t i = 0; i < m_Entries.size(); ++i) {
      if (m_Alive[i]) {
        fn(static_cast<Entity>(i), m_Entries[i].transform, m_Entries[i].sprite);
      }
    }
  }

  void Update(double deltatime);

  void Clear();

  Entity FindEntity(const std::string &name) const;

private:
  struct Entry {
    std::string name;
    Transform transform;
    Sprite sprite;
    Animator animator;
    std::optional<Particle> particle;
    std::optional<ParticleEmitter> emitter;
  };

  std::vector<Entry> m_Entries;
  std::vector<std::uint8_t> m_Alive;
  std::vector<Entity> m_Free;
  std::size_t m_LiveCount = 0;
};

} // namespace engine
