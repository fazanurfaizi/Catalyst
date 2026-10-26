#include "engine/core/GameStateManager.hpp"

#include "engine/core/Game.hpp"
#include "engine/core/GameState.hpp"

namespace engine::core {

GameStateManager::GameStateManager() = default;
GameStateManager::~GameStateManager() = default;

void GameStateManager::ChangeState(Game &game,
                                   std::unique_ptr<GameState> nextState) {
  m_pending = std::move(nextState);
  m_hasPending = true;
}

void GameStateManager::Update(Game &game, double deltaTime) {
  // Apply pending transition at the top of the frame
  if (m_hasPending) {
    if (m_current)
      m_current->OnExit(game);
    m_current = std::move(m_pending);
    m_hasPending = false;
    if (m_current)
      m_current->OnEnter(game);
  }

  if (m_current)
    m_current->OnUpdate(game, deltaTime);
}

void GameStateManager::Render(Game &game) {
  if (m_current)
    m_current->OnRender(game);
}

void GameStateManager::OnImGui(Game &game) {
  if (m_current)
    m_current->OnImGui(game);
}

void GameStateManager::OnKey(Game &game, int key, int scancode, int action,
                             int mods) {
  if (m_current)
    m_current->OnKey(game, key, scancode, action, mods);
}

void GameStateManager::OnMouseButton(Game &game, int button, int action,
                                     int mods) {
  if (m_current)
    m_current->OnMouseButton(game, button, action, mods);
}

void GameStateManager::OnMouseMove(Game &game, double x, double y) {
  if (m_current)
    m_current->OnMouseMove(game, x, y);
}

void GameStateManager::OnScroll(Game &game, double dx, double dy) {
  if (m_current)
    m_current->OnScroll(game, dx, dy);
}

void GameStateManager::OnResize(Game &game, int width, int height) {
  if (m_current)
    m_current->OnResize(game, width, height);
}

} // namespace engine::core
