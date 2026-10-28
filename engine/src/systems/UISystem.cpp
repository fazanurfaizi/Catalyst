#include "engine/systems/UISystem.hpp"
#include "engine/components/RenderLayer.hpp"
#include "engine/components/UIComponent.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/math/Rect.hpp"

#include <algorithm>
#include <vector>

namespace engine::systems {

void UIRenderSystem(ecs::Registry &reg, graphics::Renderer &renderer,
                    const platform::Display &display,
                    const graphics::TextureAtlas &atlas) {
  struct UIItem {
    ecs::Entity e;
    components::RenderLayer layer;
  };

  std::vector<UIItem> items;

  for (auto [e, rect, panel] :
       reg.View<components::UIRect, components::UIPanel>()) {
    items.push_back({e, panel.layer});
  }

  for (auto [e, rect, button] :
       reg.View<components::UIRect, components::UIButton>()) {
    items.push_back({e, components::RenderLayer::Background});
  }

  std::sort(items.begin(), items.end(),
            [](const UIItem &a, const UIItem &b) { return a.layer < b.layer; });

  renderer.Begin(display.UiProjection());
  auto *tex = &atlas.GetTexture();
  auto *whiteRegion = atlas.GetRegion("solidWhite");

  if (whiteRegion) {
    for (auto &item : items) {
      auto &rect = reg.Get<components::UIRect>(item.e);
      math::Color color;

      if (reg.Has<components::UIButton>(item.e)) {
        auto &btn = reg.Get<components::UIButton>(item.e);
        if (btn.isPressed)
          color = btn.pressedColor;
        else if (btn.isHovered)
          color = btn.hoverColor;
        else
          color = btn.normalColor;
      } else {
        color = reg.Get<components::UIPanel>(item.e).color;
      }

      glm::vec2 center = rect.position + (rect.size * 0.5f);

      renderer.Draw(center, rect.size, 0.0f, color, tex, whiteRegion->uvMin,
                    whiteRegion->uvMax,
                    static_cast<int>(components::RenderLayer::UI));
    }
  }
  renderer.End();
}

bool UIInteractionSystem(ecs::Registry &reg, const input::InputSystem &input,
                         const platform::Display &display) {
  glm::vec2 mousePos = display.CursorToFramebuffer(input.CursorPos());
  bool mouseDown = input.IsMouseDown(input::MouseButton::Left);
  bool hoveredAny = false;

  for (auto [e, rect, btn] :
       reg.View<components::UIRect, components::UIButton>()) {
    btn.wasClicked = false;
    math::Rect UiRect(rect.position.x, rect.position.y, rect.size.x,
                      rect.size.y);
    bool hover = UiRect.Contains(mousePos);

    btn.isHovered = hover;
    if (hover)
      hoveredAny = true;

    if (hover && mouseDown) {
      btn.isPressed = true;
    } else if (!mouseDown && btn.isPressed) {
      if (hover) {
        btn.wasClicked = true;
      }
      btn.isPressed = false;
    }
  }
  return hoveredAny;
}

} // namespace engine::systems
