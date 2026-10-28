#pragma once

#include "engine/ecs/Registry.hpp"
#include "engine/graphics/Renderer.hpp"
#include "engine/graphics/TextureAtlas.hpp"
#include "engine/input/InputSystem.hpp"
#include "engine/platform/Display.hpp"

namespace engine::systems {

void UIRenderSystem(ecs::Registry &reg, graphics::Renderer &renderer,
                    const platform::Display &display,
                    const graphics::TextureAtlas &atlas);

bool UIInteractionSystem(ecs::Registry &reg, const input::InputSystem &input,
                         const platform::Display &display);

} // namespace engine::systems
