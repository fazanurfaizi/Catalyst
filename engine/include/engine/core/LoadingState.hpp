#pragma once

#include "engine/asset/core/AssetHandle.hpp"
#include "engine/core/GameState.hpp"
#include <cstddef>
#include <memory>
#include <vector>

namespace engine::core {

class Game;

class LoadingState : public GameState {
public:
  explicit LoadingState(std::unique_ptr<GameState> next);

  void OnEnter(Game &game) override;
  void OnUpdate(Game &game, double deltaTime) override;
  void OnRender(Game &game) override;
  void OnImGui(Game &game) override;

private:
  std::unique_ptr<GameState> m_next;
  std::vector<asset::AssetHandle> m_toLoad;
  std::size_t m_cursor = 0;
  std::size_t m_loaded = 0;
  std::size_t m_failed = 0;
  bool m_done = false;
};
} // namespace engine::core
