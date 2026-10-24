#pragma once

#include "engine/core/Game.hpp"
#include "engine/core/GameState.hpp"
#include "engine/core/Logger.hpp"

#include "game/states/MenuState.hpp"

#include <glad/gl.h>
#include <imgui.h>

#include <cmath>

class KingdomRushGame final : public engine::Game {
public:
  KingdomRushGame()
      : engine::Game([] {
          engine::WindowConfig cfg;
          cfg.title = "Kingdom Rush Frontiers";
          return cfg;
        }()) {}

protected:
  void OnStart() override { SetState(std::make_unique<MenuState>()); }
};
