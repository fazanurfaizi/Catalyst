#pragma once

#include "engine/asset/AssetPath.hpp"
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
  void OnStart() override {
    if (auto *editor = AssetsEditor()) {
      editor->ImportAsset(engine::AssetPath("fonts/Roboto-Regular.ttf"),
                          "main_font");
      editor->ImportAsset(engine::AssetPath("audio/bg_theme.mp3"), "main_bgm");
    }
    SetState(std::make_unique<MenuState>());
  }
};
