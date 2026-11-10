#pragma once

#include "engine/animation/AnimationLibrary.hpp"
#include "engine/asset/core/AssetHandle.hpp"
#include "engine/audio/MusicPlayer.hpp"
#include "engine/core/Game.hpp"
#include "engine/core/GameState.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/graphics/TextRenderer.hpp"
#include "engine/graphics/assets/TextureAtlasAsset.hpp"
#include "engine/input/InputCodes.hpp"
#include "engine/scene/World.hpp"
#include <string_view>

class PlayingState final : public engine::core::GameState {
public:
  void OnEnter(engine::core::Game &game) override;

  void OnUpdate(engine::core::Game &game, double dt) override;
  void OnFixedUpdate(engine::core::Game &game, double dt) override;
  void OnLateUpdate(engine::core::Game &game, double dt) override;

  void OnRender(engine::core::Game &game) override;
  void OnImGui(engine::core::Game &game) override;

  void OnKey(engine::core::Game &game, engine::input::Key key, int scancode,
             engine::input::KeyAction action, int mods) override;
  void OnScroll(engine::core::Game &game, double dx, double dy) override;
  void OnMouseButton(engine::core::Game &game,
                     engine::input::MouseButton button,
                     engine::input::KeyAction, int mods) override;
  void OnMouseMove(engine::core::Game &game, double x, double y) override;
  void OnResize(engine::core::Game &game, int width, int height) override;

private:
  engine::scene::World m_world;
  engine::animation::AnimationLibrary m_animations;

  engine::graphics::TextRenderer m_textRenderer;

  std::shared_ptr<engine::graphics::FontAsset> m_font;
  std::shared_ptr<engine::graphics::TextureAtlasAsset> m_atlas;
  std::shared_ptr<engine::audio::SoundAsset> m_bgm;
  std::unique_ptr<engine::audio::MusicPlayer> m_music;

  engine::asset::AssetHandle m_fontHandle{};
  engine::asset::AssetHandle m_atlasHandle;

  engine::ecs::Entity m_camera{};
  engine::ecs::Entity m_coin{};
  engine::ecs::Entity m_tower{};
  engine::ecs::Entity m_hero{};
  engine::ecs::Entity m_enemy{};
  engine::ecs::Entity m_torch{};

  engine::ecs::Entity m_goldText{};
  engine::ecs::Entity m_waveText{};
  engine::ecs::Entity m_livesText{};

  std::uint64_t m_lastReloadGen = 0;

  void BuildUI(engine::core::Game &game);
  void BuildDemoScene(engine::core::Game &game);
  void ApplyRegion(engine::ecs::Entity e, std::string_view region);
  void DrawSceneDebug(engine::core::Game &game);
  void ReloadAssets(engine::core::Game &game);
};
