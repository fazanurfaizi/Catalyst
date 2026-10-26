#pragma once

#include "engine/core/Game.hpp"
#include "engine/core/GameState.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/graphics/Font.hpp"
#include "engine/graphics/TextRenderer.hpp"
#include "engine/graphics/TextureAtlas.hpp"

#include "game/World.hpp"
#include "game/assets/AnimationLibrary.hpp"

class PlayingState final : public engine::GameState {
public:
  void OnEnter(engine::Game &game) override;

  void OnUpdate(engine::Game &game, double dt) override;
  void OnRender(engine::Game &game) override;
  void OnImGui(engine::Game &game) override;

  void OnKey(engine::Game &game, int key, int scancode, int action,
             int mods) override;
  void OnScroll(engine::Game &game, double dx, double dy) override;
  void OnMouseButton(engine::Game &game, int button, int action,
                     int mods) override;
  void OnMouseMove(engine::Game &game, double x, double y) override;
  void OnResize(engine::Game &game, int width, int height) override;

private:
  game::World m_world;
  game::AnimationLibrary m_animations;

  engine::TextureAtlas m_atlas;
  engine::TextRenderer m_textRenderer;
  engine::Font m_font;

  engine::ecs::Entity m_camera{};
  engine::ecs::Entity m_coin{};
  engine::ecs::Entity m_tower{};
  engine::ecs::Entity m_hero{};
  engine::ecs::Entity m_enemy{};

  void BuildDemoScene(engine::Game &game);
  void ApplyRegion(engine::ecs::Entity e, const char *region);
  void DrawSceneDebug(engine::Game &game);
};
