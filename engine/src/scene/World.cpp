#include "engine/scene/World.hpp"
#include "engine/components/Name.hpp"

namespace engine::scene {

engine::ecs::Entity World::CreateEntity(const std::string &name) {
  auto e = m_registry.Create();

  m_registry.Emplace<components::Name>(e);

  m_nameToEntity.try_emplace(name, e);
  return e;
}

// void World::Update(double deltatime) {
//   float dt = static_cast<float>(deltatime);
//   for (auto [e, anim, sprite] : m_registry.View<Animator, Sprite>()) {
//     if (anim.IsActive()) {
//       anim.Advance(dt);
//       const auto &frame = anim.clip->frames[anim.currentFrame];
//       sprite.uvMin = frame.uvMin;
//       sprite.uvMax = frame.uvMax;
//     }
//   }
// }

} // namespace engine::scene
