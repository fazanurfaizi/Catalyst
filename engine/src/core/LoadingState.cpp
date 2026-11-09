#include "engine/core/LoadingState.hpp"
#include "engine/asset/cache/AssetState.hpp"
#include "engine/asset/core/AssetHandle.hpp"
#include "engine/asset/core/AssetRecord.hpp"
#include "engine/core/Game.hpp"
#include "engine/graphics/Renderer.hpp"
#include "engine/math/Rect.hpp"
#include "engine/utils/Logger.hpp"
#include "imgui.h"

#include <chrono>
#include <utility>

namespace engine::core {

namespace {
constexpr int kLeavesMs = 8;
}

LoadingState::LoadingState(std::unique_ptr<GameState> next)
    : m_next(std::move(next)) {}

void LoadingState::OnEnter(Game &game) {
  const auto &config = game.GetGameConfig();
  m_toLoad.reserve(config.assets.size());
  for (const auto &record : config.assets) {
    if (record.state == asset::AssetRecordState::Valid) {
      m_toLoad.push_back(record.handle);
    } else {
      Logger::Warn("[LoadingState] skipping '%s' (state %s)",
                   record.name.empty() ? record.path.c_str()
                                       : record.name.c_str(),
                   asset::AssetRecordStateToString(record.state));
    }
  }

  Logger::Info("[LoadingState] preloading %zu assets", m_toLoad.size());

  if (m_toLoad.empty()) {
    m_done = true;
    game.SetState(std::move(m_next));
  }
}

void LoadingState::OnUpdate(Game &game, [[maybe_unused]] double dt) {
  if (m_done)
    return;

  auto *assets = game.Assets();
  if (!assets) {
    Logger::Error("[LoadingState] no asset manager; skipping preload");
    m_done = true;
    game.SetState(std::move(m_next));
    return;
  }

  const auto start = std::chrono::steady_clock::now();

  while (m_cursor < m_toLoad.size()) {
    const asset::AssetHandle handle = m_toLoad[m_cursor++];
    assets->AcquireRaw(handle);
    Logger::Info("after Acquire: state=%s",
                 AssetLoadStateToString(assets->State(handle)));

    if (assets->State(handle) == asset::AssetLoadState::Loaded) {
      ++m_loaded;
    } else {
      ++m_failed;
      if (auto *err = assets->LastError(handle)) {
        Logger::Error("[LoadingState] failed: %s", err->ToString().c_str());
      }
    }

    const auto now = std::chrono::steady_clock::now();
    const auto elapseMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - start)
            .count();
    if (elapseMs >= kLeavesMs) {
      return;
    }
  }

  m_done = true;
  Logger::Info("[LoadingState] loaded %zu ok, %zu failed", m_loaded, m_failed);
  game.SetState(std::move(m_next));
}

void LoadingState::OnRender(Game &game) {
  auto &renderer = game.GetRenderer();
  renderer.BeginFrame(glm::mat4(1.0f),
                      engine::math::Rect(-1e4f, -1e4f, 1e4f, 1e4f));
  renderer.EndFrame();
}

void LoadingState::OnImGui(Game &game) {
  if (m_done)
    return;

  const float progress =
      m_toLoad.empty()
          ? 1.0f
          : static_cast<float>(m_cursor) / static_cast<float>(m_toLoad.size());

  // Center a small window.
  ImGuiIO &io = ImGui::GetIO();
  const ImVec2 pos{io.DisplaySize.x * 0.5f - 200.0f,
                   io.DisplaySize.y * 0.5f - 40.0f};
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(400, 80), ImGuiCond_Always);

  ImGui::Begin("##loading", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                   ImGuiWindowFlags_NoSavedSettings);

  ImGui::Text("Loading assets… %zu / %zu", m_cursor, m_toLoad.size());
  ImGui::ProgressBar(progress, ImVec2(-1, 0));
  if (m_failed > 0) {
    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%zu failed (see log)",
                       m_failed);
  }

  ImGui::End();
}

} // namespace engine::core
