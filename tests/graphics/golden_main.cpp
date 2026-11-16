#include "golden_state.hpp"

#include "engine/core/DevCapture.hpp"
#include "engine/core/Game.hpp"
#include "engine/platform/Window.hpp"

#include <cstdio>
#include <memory>
#include <string>

namespace {

class GoldenGame final : public engine::core::Game {
public:
  GoldenGame(const engine::platform::WindowConfig &win,
             const engine::core::DebugConfig &dbg,
             const std::filesystem::path &gameConfigPath,
             engine::core::DevCaptureOptions dev,
             std::filesystem::path scenePath)
      : Game(win, dbg, gameConfigPath, std::move(dev)),
        m_scenePath(std::move(scenePath)) {}

protected:
  void OnStart() override {
    if (!m_scenePath.empty()) {
      SetState(
          std::make_unique<tests::graphics::GoldenSceneState>(m_scenePath));
    }
  }

private:
  std::filesystem::path m_scenePath;
};

} // namespace

int main(int argc, char **argv) {
  engine::core::DevCaptureOptions dev;
  if (!engine::core::ParseDevCaptureArgs(argc, argv, dev)) {
    std::fprintf(stderr, "invalid dev capture arguments\n");
    return 2;
  }

  engine::platform::WindowConfig win;
  win.title = "golden";
  win.width = 1280;
  win.height = 720;
  win.vsync = false;
  win.resizable = false;

  engine::core::DebugConfig dbg;
  dbg.enableImGui = false;
  dbg.showFps = false;

  // In capture mode, the test harness insists on a fixed scene. Reject a
  // screenshot run with no scene so the golden can never drift silently.
  if (dev.IsCaptureMode() && dev.scene.empty()) {
    std::fprintf(stderr, "--screenshot requires --scene <path>\n");
    return 2;
  }

  GoldenGame game(win, dbg, "game.json", std::move(dev), dev.scene);
  return game.Run() ? 0 : 1;
}
