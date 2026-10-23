#pragma once

#include <optional>
#include <string>

#include "engine/animation/Animator.hpp"
#include "engine/components/Entity.hpp"
#include "engine/components/Particle.hpp"
#include "engine/components/ParticleEmitter.hpp"
#include "engine/components/Sprite.hpp"
#include "engine/components/Transform.hpp"

#include <glm/glm.hpp>

namespace engine {
class Texture;
}

namespace engine {

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
