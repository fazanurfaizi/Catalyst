#pragma once

#include "engine/graphics/Renderer.hpp"
#include "game/World.hpp"
#include "game/components/Sprite.hpp"
#include "game/components/Transform.hpp"
#include <algorithm>
#include <vector>

namespace game {

inline void RenderSystem(World &world, engine::Renderer &renderer) {
  auto &reg = world.Registry();

  struct RenderCmd {
    const Transform *t;
    const Sprite *s;
    float yKey;
  };

  std::vector<RenderCmd> commands;
  commands.reserve(reg.View<Transform, Sprite>().Size());

  for (auto [e, s, t] : reg.View<Sprite, Transform>()) {
    // @TODO: enable culling camera=

    const float key = (s.ySortKey != 0.0f) ? s.ySortKey : t.position.y;
    commands.push_back({&t, &s, key});
  }

  std::sort(commands.begin(), commands.end(),
            [](const RenderCmd &a, const RenderCmd &b) {
              if (a.s->layer != b.s->layer) {
                return static_cast<int>(a.s->layer) <
                       static_cast<int>(b.s->layer);
              }
              return a.yKey < b.yKey;
            });

  for (const auto &cmd : commands) {
    renderer.Draw(cmd.t->position, cmd.t->scale, cmd.t->rotation, cmd.s->color,
                  cmd.s->texture, cmd.s->uvMin, cmd.s->uvMax,
                  static_cast<int>(cmd.s->layer));
  }
}

} // namespace game
