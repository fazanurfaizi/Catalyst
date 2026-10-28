#pragma once

#include "engine/core/Game.hpp"
#include "engine/core/GameState.hpp"
#include "engine/platform/Window.hpp"
#include "engine/utils/Logger.hpp"

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
  void OnStart() override {
    if (!GetAudio().Init()) {
      engine::Logger::Error("Audio failed to initialize");
    }
    SetState(std::make_unique<MenuState>());
  }
};
