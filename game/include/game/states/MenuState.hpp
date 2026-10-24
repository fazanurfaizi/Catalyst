#pragma once

#include "engine/core/Game.hpp"
#include "engine/core/GameState.hpp"

#include "game/states/PlayingState.hpp"

#include "imgui.h"

class MenuState final : public engine::GameState {
public:
  void OnEnter(engine::Game &game) override {}
  void OnExit(engine::Game &game) override {}

  void OnUpdate(engine::Game &game, double dt) override {}
  void OnRender(engine::Game &game) override {}
  void OnImGui(engine::Game &game) override {
    ImGui::Begin("Main Menu");
    if (ImGui::Button("Play Game")) {
      game.SetState(std::make_unique<PlayingState>());
    }
    ImGui::End();
  }
  void OnMouseButton(engine::Game &game, int button, int action,
                     int mods) override {}

private:
};
