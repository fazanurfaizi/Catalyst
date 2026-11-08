#pragma once

#include "engine/core/Game.hpp"
#include "engine/core/GameState.hpp"
#include "engine/platform/Window.hpp"

#include "game/states/MenuState.hpp"

#include <glad/gl.h>
#include <imgui.h>

#include <cmath>

class KingdomRushGame final : public engine::core::Game {
public:
  KingdomRushGame()
      : engine::core::Game([] {
          engine::platform::WindowConfig cfg;
          cfg.title = "Kingdom Rush Frontiers";
          cfg.fullscreen = true;
          return cfg;
        }()) {}

protected:
  void OnStart() override { SetState(std::make_unique<MenuState>()); }
};
