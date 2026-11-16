#pragma once

#include "engine/asset/manager/IAssetManager.hpp"
#include "engine/components/TextComponent.hpp"
#include "engine/components/Transform.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/graphics/TextRenderer.hpp"
#include "engine/resources/FontAsset.hpp"

#include <glm/glm.hpp>

namespace engine::systems {

inline void TextRenderSystem(ecs::Registry &reg, asset::IAssetManager &assets,
                             graphics::TextRenderer &renderer,
                             const glm::mat4 &projection) {
  for (auto [e, text, transform] :
       reg.View<components::TextComponent, components::Transform>()) {
    if (text.font.IsNull() || text.text.empty()) {
      continue;
    }

    auto asset = assets.Acquire<resources::FontAsset>(text.font);
    if (!asset || !asset->EnsureGpu()) {
      continue;
    }

    renderer.Draw(*asset, text.text, transform.position, text.scale,
                  text.color.toVec4(), projection, text.maxWidth,
                  text.alignment);
  }
}

} // namespace engine::systems
