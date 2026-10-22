#pragma once

#include "engine/animation/Animator.hpp"
#include <cstdint>
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace engine {
class Texture;
}

namespace engine {

using Entity = std::uint32_t;
constexpr Entity kInvalidEntity = 0xFFFFFFFFu;

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

  // fn(Entity, const Transform&, const Sprite&)
  template <typename Fn> void ForEach(Fn &&fn) const {
    for (std::size_t i = 0; i < m_Entries.size(); ++i) {
      if (m_Alive[i]) {
        fn(static_cast<Entity>(i), m_Entries[i].transform, m_Entries[i].sprite);
      }
    }
  }

  void Update(double deltatime);

private:
  struct Entry {
    std::string name;
    Transform transform;
    Sprite sprite;
    Animator animator;
  };

  std::vector<Entry> m_Entries;
  std::vector<std::uint8_t> m_Alive;
  std::vector<Entity> m_Free;
  std::size_t m_LiveCount = 0;
};

} // namespace engine
