#include "game/states/PlayingState.hpp"

#include "engine/assets/AssetPath.hpp"
#include "engine/components/Sprite.hpp"
#include "engine/components/Transform.hpp"
#include "engine/core/Game.hpp"
#include "engine/graphics/Renderer.hpp"
#include "engine/graphics/TextureAtlas.hpp"
#include "engine/math/Color.hpp"
#include "engine/scene/SceneSerializer.hpp"
#include "engine/utils/Logger.hpp"

#include "engine/components/Animator.hpp"
#include "engine/components/Camera2D.hpp"
#include "engine/components/RenderLayer.hpp"
#include "engine/components/Velocity.hpp"
#include "engine/systems/AnimationSystem.hpp"
#include "engine/systems/CameraFollowSystem.hpp"
#include "engine/systems/MovementSystem.hpp"
#include "engine/systems/ParticleSystem.hpp"
#include "engine/systems/RenderSystem.hpp"

#include <cmath>
#include <cstdlib>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <imgui.h>

namespace {
void DrawWorldGrid(engine::graphics::Renderer &renderer,
                   const engine::components::Camera2D &camera,
                   const engine::graphics::TextureAtlas &atlas) {
  auto *region = atlas.GetRegion("solidWhite");
  if (!region)
    return;

  // Visible world bounds: compute from screen corners.
  const auto bounds = camera.VisibleWorldBounds();

  const int x0 = static_cast<int>(std::floor(bounds.minX));
  const int x1 = static_cast<int>(std::ceil(bounds.maxX));
  const int y0 = static_cast<int>(std::floor(bounds.minY));
  const int y1 = static_cast<int>(std::ceil(bounds.maxY));

  constexpr int kMaxCells = 4096;
  const int cols = x1 - x0;
  const int rows = y1 - y0;
  if (cols * rows > kMaxCells)
    return;

  const engine::math::Color evenCell{0.13f, 0.17f, 0.24f, 1.0f};
  const engine::math::Color oddCell{0.16f, 0.20f, 0.28f, 1.0f};
  const engine::math::Color axisCell{0.20f, 0.24f, 0.34f, 1.0f};
  const auto *tex = &atlas.GetTexture();

  for (int y = y0; y < y1; ++y) {
    for (int x = x0; x < x1; ++x) {
      engine::math::Color color;
      if (x == 0 || y == 0)
        color = axisCell;
      else
        color = ((x + y) & 1) ? oddCell : evenCell;

      renderer.Draw({static_cast<float>(x) + 0.5f,
                     static_cast<float>(y) + 0.5f}, // position
                    {1.0f, 1.0f},                   // scale
                    0.0f,                           // rotation
                    color, tex, region->uvMin, region->uvMax,
                    static_cast<int>(engine::components::RenderLayer::Ground));
    }
  }
}
} // namespace

void PlayingState::OnEnter(engine::core::Game &game) {
  engine::Logger::Info("OnInitialize");

  auto &reg = m_world.Registry();
  m_camera = m_world.CreateEntity("MainCamera");

  game.GetWindow().ClearWindow();

  auto &cam = reg.Emplace<engine::components::Camera2D>(m_camera);
  cam.SetDisplay(&game.Display());
  cam.orthoSize = 10.0f;
  cam.active = true;

  const int loaded = m_atlas.AddFromDirectory(engine::AssetPath("sprites"));
  m_font.LoadFromFile(engine::AssetPath("fonts/Roboto-Regular.ttf"), 32.0f);

  m_atlas.AddSolid("solidWhite", 0xFFFFFFFF, 16);

  if (!m_atlas.HasRegion("checkerRed"))
    m_atlas.AddCheckerboard("checkerRed", 8, 0xCC3333FF, 0x661111FF);
  if (!m_atlas.HasRegion("checkerBlue"))
    m_atlas.AddCheckerboard("checkerBlue", 8, 0x3366CCFF, 0x112244FF);
  if (!m_atlas.HasSource("tower"))
    m_atlas.AddSolid("tower", 0xE65555FF, 16);
  if (!m_atlas.HasSource("enemy"))
    m_atlas.AddCheckerboard("enemy", 8, 0x3366CCFF, 0x112244FF);

  if (!m_atlas.HasSource("hero_idle_0"))
    for (int i = 0; i < 10; ++i)
      m_atlas.AddCheckerboard("hero_idle_" + std::to_string(i), 9 + i,
                              0xCC3333FF, 0x661111FF);

  if (!m_atlas.HasSource("hero_attack_0"))
    for (int i = 0; i < 10; ++i)
      m_atlas.AddCheckerboard("hero_attack_" + std::to_string(i), 9 + i,
                              0xFFDD66FF, 0xAA8800FF);

  if (!m_atlas.HasSource("coin_0")) {
    for (int i = 0; i < 6; ++i) {
      m_atlas.AddCheckerboard("coin_" + std::to_string(i), 4 + 1, 0xFFDD66FF,
                              0xAA8800FF);
    }
  }

  if (!m_atlas.Pack()) {
    engine::Logger::Error("Atlas pack failed");
  }

  m_animations.AddStrip("coin_spin", m_atlas, "coin", 6, 0.2f,
                        /*loop=*/true);

  m_animations.AddStrip("hero_idle", m_atlas, "hero_idle", 10, 0.15f, true);
  m_animations.AddStrip("hero_attack", m_atlas, "hero_attack", 10, 0.08f,
                        false);
  m_animations.AddStrip("hero_move", m_atlas, "hero_move", 10, 0.1f, true);
  m_animations.AddStrip("coin_spin", m_atlas, "coin", 6, 0.08f, true);

  game.GetAudio().PlayMusic(engine::AssetPath("audio/bg_theme.mp3"));

  engine::Logger::Info(
      loaded > 0
          ? "[Game] using %d real PNGs + fallbacks"
          : "[Game] no assets/sprites/*.png — using procedural fallbacks",
      loaded);

  BuildDemoScene(game);
}

void PlayingState::OnUpdate(engine::core::Game &game, double dt) {
  auto &reg = m_world.Registry();
  auto &camera = reg.Get<engine::components::Camera2D>(m_camera);
  auto &in = game.Input();

  const float speed = 5.0f * static_cast<float>(dt);

  if (in.IsKeyDown(engine::input::Key::W))
    camera.Move({0.0f, speed});
  if (in.IsKeyDown(engine::input::Key::S))
    camera.Move({0.0f, -speed});
  if (in.IsKeyDown(engine::input::Key::A))
    camera.Move({-speed, 0.0f});
  if (in.IsKeyDown(engine::input::Key::D))
    camera.Move({speed, 0.0f});
  if (in.IsKeyDown(engine::input::Key::Q))
    camera.RotateBy(+0.5f * static_cast<float>(dt));
  if (in.IsKeyDown(engine::input::Key::E))
    camera.RotateBy(-0.5f * static_cast<float>(dt));

  if (reg.IsAlive(m_hero)) {
    auto &a = reg.Get<engine::components::Animator>(m_hero);
    auto &t = reg.Get<engine::components::Transform>(m_hero);
    auto &v = reg.Get<engine::components::Velocity>(m_hero);

    auto *idleClip = m_animations.Get("hero_idle");
    auto *attackClip = m_animations.Get("hero_attack");
    auto *moveClip = m_animations.Get("hero_move");

    if (idleClip && attackClip && a.finished && a.clip == attackClip) {
      a.Play(idleClip);
    }

    if (!v.ArrivedAtTarget(t.position)) {
      if (idleClip && a.clip != attackClip && a.clip != moveClip && moveClip) {
        a.Play(moveClip);
      }
    } else {
      t.position = v.targetPosition;

      if (idleClip && a.clip != attackClip && a.clip != idleClip) {
        a.Play(idleClip);
      }
    }
  }

  // Enemy orbits origin.
  if (reg.IsAlive(m_enemy)) {
    const float angle = static_cast<float>(game.ElapsedTime());
    auto &t = reg.Get<engine::components::Transform>(m_enemy);
    t.position = {std::cos(angle) * 6.0f, std::sin(angle) * 4.0f};
    t.rotation = angle;
  }

  engine::systems::AnimationSystem(reg, dt);
  engine::systems::ParticleSystem(reg, dt);
  engine::systems::MoveTowardTargetSytem(reg, dt);
  engine::systems::CameraFollowSystem(reg, dt);

  reg.FlushDeferred();
}

void PlayingState::OnRender(engine::core::Game &game) {
  auto &renderer = game.GetRenderer();
  auto &reg = m_world.Registry();
  auto &camera = reg.Get<engine::components::Camera2D>(m_camera);

  renderer.Begin(camera.ViewProjectionMatrix());

  DrawWorldGrid(renderer, camera, m_atlas);

  engine::systems::RenderSystem(m_world, renderer, camera);

  renderer.End();

  m_textRenderer.Draw(m_font, "Gold: 100", {60.0f, 100.0f}, 2.0f,
                      {1.0f, 0.85f, 0.2f, 1.0f}, game.Display().UiProjection());
}

void PlayingState::OnImGui(engine::core::Game &game) {
#ifndef DEBUG
  DrawSceneDebug(game);
#endif // DEBUG
}

void PlayingState::OnKey(engine::core::Game &game, engine::input::Key key,
                         int scancode, engine::input::KeyAction action,
                         int mods) {
  (void)scancode;
  (void)mods;

  if (action != engine::input::KeyAction::Press)
    return;

  if (key == engine::input::Key::Escape)
    game.GetWindow().RequestClose();

  if (key == engine::input::Key::F11)
    game.ToggleFullscreen();

  if (key == engine::input::Key::Space)
    engine::Logger::Info("Space tapped");
}

void PlayingState::OnScroll(engine::core::Game &game, double dx, double dy) {
  (void)dx;
  // Zoom around the cursor for natural feel.
  auto &reg = m_world.Registry();
  auto &camera = reg.Get<engine::components::Camera2D>(m_camera);

  auto cursor = game.Input().CursorPos();
  const auto before = camera.ScreenToWorldFromCursor({cursor.x, cursor.y});
  camera.ZoomBy(dy > 0 ? 1.1f : 1.0f / 1.1f);
  const auto after = camera.ScreenToWorldFromCursor({cursor.x, cursor.y});
  const auto delta = before - after;
  camera.Move({delta.x, delta.y});
}

void PlayingState::OnMouseButton(engine::core::Game &game,
                                 engine::input::MouseButton button,
                                 engine::input::KeyAction action, int mods) {
  (void)mods;
  if (action != engine::input::KeyAction::Press)
    return;

  auto &reg = m_world.Registry();
  auto &camera = reg.Get<engine::components::Camera2D>(m_camera);
  auto &in = game.Input();

  glm::vec2 worldPos = camera.ScreenToWorldFromCursor(in.CursorPos());

  if (button == engine::input::MouseButton::Left) {
    auto &animation = reg.Get<engine::components::Animator>(m_hero);
    auto *idleClip = m_animations.Get("hero_idle");
    auto *attackClip = m_animations.Get("hero_attack");
    if (idleClip && attackClip && animation.clip == idleClip) {
      animation.Play(attackClip);
    }
  }

  if (button == engine::input::MouseButton::Right) {
    if (auto *val = reg.TryGet<engine::components::Velocity>(m_hero)) {
      val->targetPosition = worldPos;
      engine::Logger::Info("target=(%.2f,%.2f) speed=%.2f", worldPos.x,
                           worldPos.y, val->speed);
    }
  }
}

void PlayingState::OnMouseMove(engine::core::Game &game, double x, double y) {
  (void)x;
  (void)y;
}

void PlayingState::OnResize(engine::core::Game &game, int w, int h) {
  engine::Logger::Info("Resized to %d x %d", w, h);
}

void PlayingState::BuildDemoScene(engine::core::Game &game) {
  auto &reg = m_world.Registry();
  auto &cam = reg.Get<engine::components::Camera2D>(m_camera);

  // Tower
  m_tower = m_world.CreateEntity("Tower");
  reg.Emplace<engine::components::Transform>(m_tower,
                                             engine::components::Transform{
                                                 .position = {-4.0f, -2.0f},
                                                 .scale = {1.5f, 1.5f},
                                             });
  reg.Emplace<engine::components::Sprite>(
      m_tower, engine::components::Sprite{
                   .layer = engine::components::RenderLayer::Entities});
  ApplyRegion(m_tower, "tower");

  // Hero
  m_hero = m_world.CreateEntity("Hero");
  reg.Emplace<engine::components::Transform>(
      m_hero, engine::components::Transform{.position = {4.0f, -2.0f},
                                            .scale = {1.2f, 1.7f}});
  reg.Emplace<engine::components::Sprite>(
      m_hero, engine::components::Sprite{
                  .texture = &m_atlas.GetTexture(),
                  .layer = engine::components::RenderLayer::Entities});
  reg.Emplace<engine::components::Velocity>(m_hero,
                                            engine::components::Velocity{
                                                .targetPosition = {4.0f, -2.0f},
                                                .speed = 5.0f,
                                            });
  reg.Emplace<engine::components::Animator>(m_hero);
  ApplyRegion(m_hero, "hero_idle_0");
  cam.followTarget = m_hero;

  // Enemy
  m_enemy = m_world.CreateEntity("Enemy");
  reg.Emplace<engine::components::Transform>(
      m_enemy, engine::components::Transform{.scale = {0.9f, 1.0f}});
  reg.Emplace<engine::components::Sprite>(
      m_enemy, engine::components::Sprite{
                   .layer = engine::components::RenderLayer::Entities});
  ApplyRegion(m_enemy, "enemy");

  // Coin
  m_coin = m_world.CreateEntity("Coin");
  reg.Emplace<engine::components::Transform>(
      m_coin, engine::components::Transform{.position = {3.0f, 3.0f},
                                            .scale = {0.8f, 0.8f}});
  reg.Emplace<engine::components::Sprite>(
      m_coin, engine::components::Sprite{
                  .color = engine::math::kColorWhite,
                  .texture = &m_atlas.GetTexture(),
                  .layer = engine::components::RenderLayer::Ground});
  reg.Emplace<engine::components::Animator>(m_coin);
  ApplyRegion(m_coin, "coin_0");

  if (auto *idleClip = m_animations.Get("hero_idle"))
    reg.Get<engine::components::Animator>(m_hero).Play(idleClip);
  if (auto *coinClip = m_animations.Get("coin_spin"))
    reg.Get<engine::components::Animator>(m_coin).Play(coinClip);
}

void PlayingState::ApplyRegion(engine::ecs::Entity e, const char *region) {
  auto *r = m_atlas.GetRegion(region);
  if (!r) {
    engine::Logger::Warn("[Game] unknown region: %s", region);
    return;
  }
  auto &reg = m_world.Registry();
  auto &s = reg.Get<engine::components::Sprite>(e);

  s.texture = &m_atlas.GetTexture();
  s.uvMin = r->uvMin;
  s.uvMax = r->uvMax;
  // s.regionId =
}

void PlayingState::DrawSceneDebug(engine::core::Game &game) {
  auto &reg = m_world.Registry();

  ImGui::Begin("Renderer");
  ImGui::Text("Draw calls: %u", game.GetRenderer().DrawCalls());
  ImGui::Text("Quads:      %u", game.GetRenderer().QuadCount());
  // ImGui::Text("Culled:     %u", game.GetRenderer().CulledCount());
  // ImGui::Text("Alive:      %zu", m_scene.Size());
  // ImGui::Text("Clips: %d", m_animations.Count());

  bool fullscreen = game.IsFullscreen();
  if (ImGui::Checkbox("Fullscreen", &fullscreen)) {
    game.SetFullscreen(fullscreen);
  }

  const auto ws = game.Display().WindowSize();
  ImGui::Text("Window: %d x %d", ws.x, ws.y);
  ImGui::Text("Framebuffer: %d x %d", game.Display().FramebufferSize().x,
              game.Display().FramebufferSize().y);

  if (!game.IsFullscreen()) {
    if (ImGui::Button("1280x720"))
      game.SetWindowedSize(1280, 720);
    ImGui::SameLine();
    if (ImGui::Button("1600x900"))
      game.SetWindowedSize(1600, 900);
    ImGui::SameLine();
    if (ImGui::Button("1920x1080"))
      game.SetWindowedSize(1920, 1080);
  }

  ImGui::Separator();

  auto &camera = reg.Get<engine::components::Camera2D>(m_camera);
  auto p = camera.position;
  ImGui::Text("Cam pos: (%.2f, %.2f)", p.x, p.y);
  ImGui::Text("Cam zoom: %.2f", camera.zoom);
  ImGui::Text("Rot:  %.2f rad", camera.rotation);
  ImGui::Text("Ortho half-height: %.2f", camera.orthoSize);
  ImGui::Separator();
  ImGui::Text("WASD  move");
  ImGui::Text("Q / E rotate");
  ImGui::Text("Wheel zoom at cursor");

  ImGui::Separator();

  if (ImGui::Button("Spawn Particle Emitter")) {
    auto e = m_world.CreateEntity("Emitter");
    reg.Emplace<engine::components::Transform>(
        e,
        engine::components::Transform{
            .position = {(float)(rand() % 20 - 10), (float)(rand() % 14 - 7)}});

    reg.Emplace<engine::components::ParticleEmitter>(
        e, engine::components::ParticleEmitter{
               .emitRate = 200.0f,
               .duration = 1.5f,
               .minLife = 0.5f,
               .maxLife = 1.5f,
               .minSpeed = 1.0f,
               .maxSpeed = 5.0f,
               .minScale = 0.1f,
               .maxScale = 0.3f,
               .startColor = {1.0f, 0.8f, 0.2f, 1.0f},
               .endColor = {1.0f, 0.0f, 0.0f, 0.0f},
               .texture = &m_atlas.GetTexture()});

    if (auto *r = m_atlas.GetRegion("solidWhite")) {
      reg.Get<engine::components::ParticleEmitter>(e).uvMin = r->uvMin;
      reg.Get<engine::components::ParticleEmitter>(e).uvMax = r->uvMax;
    }
  }

  ImGui::Separator();

  ImGui::End();
}
