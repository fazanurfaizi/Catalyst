#pragma once

#include <cstdint>

namespace game {

enum class RenderLayer : std::uint8_t {
  Background = 0, // grid, sky, parallax
  Ground = 1,     // floor tiles, decor
  Shadows = 2,    // drop shadows
  Entities = 3,   // heroes, enemies, towers (default)
  Overlay = 4,    // effects, particles on top
  UI = 5,         // HUD, health bars
  COUNT,
};

}
