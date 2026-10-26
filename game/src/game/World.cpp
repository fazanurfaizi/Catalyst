#include "game/World.hpp"
#include "game/components/Animator.hpp"
#include "game/components/Name.hpp"
#include "game/components/Sprite.hpp"
#include "game/components/Transform.hpp"

namespace game {

engine::ecs::Entity World::CreateEntity(const std::string &name) {
  auto e = m_registry.Create();

  m_registry.Emplace<game::Transform>(e);
  m_registry.Emplace<game::Sprite>(e);
  m_registry.Emplace<game::Animator>(e);
  m_registry.Emplace<game::Name>(e);

  m_nameToEntity.try_emplace(name, e);
  return e;
}

void World::Update(double deltatime) {
  float dt = static_cast<float>(deltatime);
  for (auto [e, anim, sprite] : m_registry.View<Animator, Sprite>()) {
    if (anim.IsActive()) {
      anim.Advance(dt);
      const auto &frame = anim.clip->frames[anim.currentFrame];
      sprite.uvMin = frame.uvMin;
      sprite.uvMax = frame.uvMax;
    }
  }
}

} // namespace game
