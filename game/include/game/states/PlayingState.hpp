#pragma once

#include "engine/animation/AnimationLibrary.hpp"
#include "engine/asset/core/AssetHandle.hpp"
#include "engine/audio/MusicPlayer.hpp"
#include "engine/core/Game.hpp"
#include "engine/core/GameState.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/graphics/TextRenderer.hpp"
#include "engine/input/InputCodes.hpp"
#include "engine/resources/TextureAtlasAsset.hpp"
#include "engine/scene/Scene.hpp"

#include <cstdint>
#include <memory>
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
  engine::scene::Scene m_scene;
  engine::animation::AnimationLibrary m_animations;
  engine::graphics::TextRenderer m_textRenderer;
  std::unique_ptr<engine::audio::MusicPlayer> m_music;

  engine::asset::AssetHandle m_fontHandle{};
  engine::asset::AssetHandle m_atlasHandle{};
  engine::asset::AssetHandle m_bgmHandle{};
  engine::asset::AssetHandle m_heroAttackSfxHandle{};

  std::shared_ptr<engine::resources::TextureAtlasAsset> m_atlasCache;

  std::uint64_t m_lastReloadGen = 0;

#ifndef DEBUG
  char m_importPath[256] = "textures/tower.png";
#endif // !DEBUG

  void ResolveHandles(engine::core::Game &game);
  void BuildAnimations();
  void ApplyRegion(engine::ecs::Entity e, std::string_view region);
  void DrawSceneDebug(engine::core::Game &game);
  void ReloadAssets(engine::core::Game &game);
};
