#pragma once

#include "engine/components/Camera2D.hpp"
#include "engine/components/Sprite.hpp"
#include "engine/components/Transform.hpp"
#include "engine/graphics/Renderer.hpp"
#include "engine/scene/World.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

namespace engine::systems {

inline void RenderSystem(scene::World &world, graphics::Renderer &renderer,
                         const components::Camera2D camera) {
  auto &reg = world.Registry();

  struct RenderCmd {
    const components::Transform *t;
    const components::Sprite *s;
    float yKey;
  };

  std::vector<RenderCmd> commands;
  commands.reserve(
      reg.View<components::Transform, components::Sprite>().Size());

  const auto viewRect = camera.VisibleWorldBounds();

  for (auto [e, s, t] : reg.View<components::Sprite, components::Transform>()) {
    const float hx = t.scale.x * 0.5;
    const float hy = t.scale.y * 0.5;
    float ex = hx;
    float ey = hy;

    if (t.rotation != 0.0f) {
      const float c = std::cos(t.rotation);
      const float sn = std::sin(t.rotation);
      ex = std::abs(hx * c) + std::abs(hy * sn);
      ey = std::abs(hx * sn) + std::abs(hy * c);
    }

    math::Rect entityRect(t.position.x - ex, t.position.y - ey, ex * 2.0f,
                          ey * 2.0f);

    if (!viewRect.Intersects(entityRect)) {
      continue;
    }

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

} // namespace engine::systems
