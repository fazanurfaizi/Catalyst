#pragma once

#include "engine/core/Game.hpp"
#include "engine/core/GameState.hpp"

#include "engine/input/InputCodes.hpp"
#include "game/states/PlayingState.hpp"

#include "imgui.h"

class MenuState final : public engine::core::GameState {
public:
  void OnEnter(engine::core::Game &game) override {}
  void OnExit(engine::core::Game &game) override {}

  void OnUpdate(engine::core::Game &game, double dt) override {}
  void OnRender(engine::core::Game &game) override {}
  void OnImGui(engine::core::Game &game) override {
    ImGui::Begin("Main Menu");
    if (ImGui::Button("Play Game")) {
      game.SetState(std::make_unique<PlayingState>());
    }
    ImGui::End();
  }
  void OnMouseButton(engine::core::Game &game,
                     engine::input::MouseButton button,
                     engine::input::KeyAction action, int mods) override {}

private:
};
