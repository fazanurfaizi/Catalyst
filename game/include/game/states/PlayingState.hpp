#pragma once

#include "engine/animation/AnimationLibrary.hpp"
#include "engine/core/Game.hpp"
#include "engine/core/GameState.hpp"
#include "engine/graphics/Font.hpp"
#include "engine/graphics/TextRenderer.hpp"
#include "engine/graphics/TextureAtlas.hpp"
#include "engine/scene/Scene.hpp"

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
  engine::Scene m_scene;
  engine::TextureAtlas m_atlas;
  engine::AnimationLibrary m_animations;
  engine::TextRenderer m_textRenderer;
  engine::Font m_font;

  engine::Entity m_coin = engine::kInvalidEntity;
  engine::Entity m_tower = engine::kInvalidEntity;
  engine::Entity m_hero = engine::kInvalidEntity;
  engine::Entity m_enemy = engine::kInvalidEntity;

  void BuildDemoScene(engine::Game &game);
  void ApplyRegion(engine::Entity e, const char *region);
  void DrawSceneDebug(engine::Game &game);
};
