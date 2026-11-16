#pragma once

#include "engine/core/GameState.hpp"
#include "engine/scene/Scene.hpp"

#include <filesystem>

namespace tests::graphics {

// GameState that loads a fixed, deterministic scene from a scene file and
// renders it through the normal Renderer path. No update, no animation, no
// ImGui.
class GoldenSceneState : public engine::core::GameState {
public:
  explicit GoldenSceneState(std::filesystem::path scenePath);

  void OnEnter(engine::core::Game &game) override;
  void OnExit(engine::core::Game &game) override;
  void OnFixedUpdate(engine::core::Game &game, double dt) override;
  void OnUpdate(engine::core::Game &game, double dt) override;
  void OnLateUpdate(engine::core::Game &game, double dt) override;
  void OnRender(engine::core::Game &game) override;
  void OnImGui(engine::core::Game &game) override;

private:
  std::filesystem::path m_scenePath;
  engine::scene::Scene m_scene;
  bool m_loaded = false;
};

} // namespace tests::graphics
