#include "golden_state.hpp"

#include "engine/components/Light.hpp"
#include "engine/components/Sprite.hpp"
#include "engine/components/Transform.hpp"
#include "engine/core/Game.hpp"
#include "engine/graphics/Renderer.hpp"
#include "engine/math/Rect.hpp"
#include "engine/scene/SceneSerializer.hpp"
#include "engine/utils/Logger.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace tests::graphics {

namespace {
engine::math::Camera MakeGoldenCamera(int width, int height) {
  engine::math::Camera cam{};
  cam.viewProjection =
      glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height),
                 0.0f, -1.0f, 1.0f);
  cam.visibleBounds = engine::math::Rect(0.0f, 0.0f, static_cast<float>(width),
                                         static_cast<float>(height));
  return cam;
}
} // namespace

GoldenSceneState::GoldenSceneState(std::filesystem::path scenePath)
    : m_scenePath(std::move(scenePath)) {}

void GoldenSceneState::OnEnter(engine::core::Game & /*game*/) {
  const std::string path = m_scenePath.string();

  if (!engine::scene::SceneSerializer::Load(m_scene, path.c_str())) {
    engine::Logger::Error("[GoldenState] failed to load scene '%s'",
                          path.c_str());
    m_loaded = false;
    return;
  }

  auto &reg = m_scene.Registry();

  // Sanity check: golden scenes must produce exactly what the branch under
  // test expects. Empty is valid (case D), but a sprite scene that loaded
  // zero entities silently turns case A into case D. Catch it here, not
  // in the PPM diff.
  std::size_t entityCount = 0;
  for (auto _ : reg.View<engine::components::Transform>()) {
    (void)_;
    ++entityCount;
  }
  engine::Logger::Info("[GoldenState] loaded '%s' (%zu transforms)",
                       path.c_str(), entityCount);

  // Per-case expectations. Keep these in sync with tests/graphics/scenes/.
  const std::string name = m_scenePath.filename().string();
  const bool wantEmpty = (name == "empty.json");
  const bool wantSprites =
      (name == "sprites_only.json" || name == "sprites_lights.json");
  const bool wantLights =
      (name == "lights_only.json" || name == "sprites_lights.json");

  const bool hasSprites = reg.View<engine::components::Sprite>().Size() > 0;
  const bool hasLights =
      reg.View<engine::components::PointLight>().Size() > 0 ||
      reg.View<engine::components::DirectionalLight>().Size() > 0 ||
      reg.View<engine::components::SpotLight>().Size() > 0;

  if (wantEmpty && (hasSprites || hasLights)) {
    engine::Logger::Error(
        "[GoldenState] empty.json must contain no sprites or lights");
    m_loaded = false;
    return;
  }
  if (wantSprites && !hasSprites) {
    engine::Logger::Error("[GoldenState] '%s' must contain sprites",
                          path.c_str());
    m_loaded = false;
    return;
  }
  if (wantLights && !hasLights) {
    engine::Logger::Error("[GoldenState] '%s' must contain lights",
                          path.c_str());
    m_loaded = false;
    return;
  }

  m_loaded = true;
}

void GoldenSceneState::OnExit(engine::core::Game & /*game*/) {}

void GoldenSceneState::OnFixedUpdate(engine::core::Game & /*game*/,
                                     double /*dt*/) {}
void GoldenSceneState::OnUpdate(engine::core::Game & /*game*/, double /*dt*/) {}
void GoldenSceneState::OnLateUpdate(engine::core::Game & /*game*/,
                                    double /*dt*/) {}

void GoldenSceneState::OnRender(engine::core::Game &game) {
  if (!m_loaded)
    return;

  auto &renderer = game.GetRenderer();
  const int w = renderer.Width();
  const int h = renderer.Height();

  auto camera = MakeGoldenCamera(w, h);
  renderer.BeginFrame(camera);

  if (game.Assets()) {
    renderer.RenderSprites(m_scene, game.Assets());
  }
  renderer.RenderLights(m_scene);

  renderer.EndFrame();
}

void GoldenSceneState::OnImGui(engine::core::Game & /*game*/) {}

} // namespace tests::graphics
