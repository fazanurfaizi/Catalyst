#include "game/states/PlayingState.hpp"

#include "GLFW/glfw3.h"

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
#include "glm/ext/matrix_clip_space.hpp"

#include <algorithm>
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
  const auto tl = camera.ScreenToWorld({0.0f, 0.0f});
  const auto br =
      camera.ScreenToWorld({static_cast<float>(camera.viewportWidth),
                            static_cast<float>(camera.viewportHeight)});
  const float minX = std::min(tl.x, br.x);
  const float maxX = std::max(tl.x, br.x);
  const float minY = std::min(tl.y, br.y);
  const float maxY = std::max(tl.y, br.y);

  const int x0 = static_cast<int>(std::floor(minX));
  const int x1 = static_cast<int>(std::ceil(maxX));
  const int y0 = static_cast<int>(std::floor(minY));
  const int y1 = static_cast<int>(std::ceil(maxY));

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

  game.SetClearColor({0.15f, 0.20f, 0.30f, 1.0f});

  auto &cam = reg.Emplace<engine::components::Camera2D>(m_camera);
  cam.SetViewport(game.Config().width, game.Config().height);
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

  const float speed = 5.0f * static_cast<float>(dt);

  if (game.IsKeyDown(GLFW_KEY_W))
    camera.Move({0.0f, speed});
  if (game.IsKeyDown(GLFW_KEY_S))
    camera.Move({0.0f, -speed});
  if (game.IsKeyDown(GLFW_KEY_A))
    camera.Move({-speed, 0.0f});
  if (game.IsKeyDown(GLFW_KEY_D))
    camera.Move({speed, 0.0f});
  if (game.IsKeyDown(GLFW_KEY_Q))
    camera.RotateBy(+0.5f * static_cast<float>(dt));
  if (game.IsKeyDown(GLFW_KEY_E))
    camera.RotateBy(-0.5f * static_cast<float>(dt));

  if (game.IsKeyDown(GLFW_KEY_ESCAPE))
    glfwSetWindowShouldClose(game.Window(), GLFW_TRUE);

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
    const float angle = static_cast<float>(glfwGetTime());
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

  glm::mat4 viewProj = camera.ViewProjectionMatrix();

  game.SetClearColor({0.10f, 0.12f, 0.16f, 1.0f});

  renderer.Begin(viewProj);

  DrawWorldGrid(renderer, camera, m_atlas);

  engine::systems::RenderSystem(m_world, renderer, camera);

  renderer.End();

  glm::mat4 uiProjection =
      glm::ortho(0.0f, (float)camera.viewportWidth,
                 (float)camera.viewportHeight, 0.0f, -1.0f, 1.0f);

  m_textRenderer.Draw(m_font, "Gold: 100", {60.0f, 100.0f}, 2.0f,
                      {1.0f, 0.85f, 0.2f, 1.0f}, uiProjection);
}

void PlayingState::OnImGui(engine::core::Game &game) {
#ifndef DEBUG
  DrawSceneDebug(game);
#endif // DEBUG
}

void PlayingState::OnKey(engine::core::Game &game, int key, int scancode,
                         int action, int mods) {
  (void)scancode;
  (void)mods;
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(game.Window(), GLFW_TRUE);
  }
  if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
    engine::Logger::Info("Space tapped");
  }
}

void PlayingState::OnScroll(engine::core::Game &game, double dx, double dy) {
  (void)dx;
  // Zoom around the cursor for natural feel.
  auto &reg = m_world.Registry();
  auto &camera = reg.Get<engine::components::Camera2D>(m_camera);
  double mx, my;
  glfwGetCursorPos(game.Window(), &mx, &my);
  const auto before =
      camera.ScreenToWorld({static_cast<float>(mx), static_cast<float>(my)});
  camera.ZoomBy(dy > 0 ? 1.1f : 1.0f / 1.1f);
  const auto after =
      camera.ScreenToWorld({static_cast<float>(mx), static_cast<float>(my)});
  const auto delta = before - after;
  camera.Move({delta.x, delta.y});
}

void PlayingState::OnMouseButton(engine::core::Game &game, int button,
                                 int action, int mods) {
  (void)mods;
  auto &reg = m_world.Registry();
  auto &camera = reg.Get<engine::components::Camera2D>(m_camera);

  double mx, my;
  glfwGetCursorPos(game.Window(), &mx, &my);

  glm::vec2 worldPos =
      camera.ScreenToWorld({static_cast<float>(mx), static_cast<float>(my)});

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    auto &animation = reg.Get<engine::components::Animator>(m_hero);
    auto *idleClip = m_animations.Get("hero_idle");
    auto *attackClip = m_animations.Get("hero_attack");
    if (idleClip && attackClip && animation.clip == idleClip) {
      animation.Play(attackClip);
    }
  }

  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    if (auto *val = reg.TryGet<engine::components::Velocity>(m_hero)) {
      val->targetPosition = worldPos;
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
  auto &cam = reg.Emplace<engine::components::Camera2D>(m_camera);

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
  reg.Emplace<engine::components::Velocity>(m_hero);
  reg.Emplace<engine::components::Animator>(m_hero);
  ApplyRegion(m_hero, "hero_idle_0");
  cam.SetViewport(game.Config().width, game.Config().height);
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

  // bool cullOn = game.GetRenderer().CullingEnabled();
  // if (ImGui::Checkbox("Culling", &cullOn)) {
  //   game.GetRenderer().SetCullingEnabled(cullOn);
  // }

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

  // if (ImGui::Button("Spawn Particle Emitter")) {
  //   auto e = m_scene.CreateEntity("Emitter");
  //   m_scene.TransformOf(e).position = {(float)(rand() % 20 - 10),
  //                                      (float)(rand() % 14 - 7)};
  //
  //   auto &emit = m_scene.EmitterOf(e).emplace();
  //   emit.emitRate = 200.0f;
  //   emit.minLife = 0.5f;
  //   emit.maxLife = 1.5f;
  //   emit.minSpeed = 1.0f;
  //   emit.maxSpeed = 5.0f;
  //   emit.minScale = 0.1f;
  //   emit.maxScale = 0.3f;
  //   emit.duration = 1.5f;
  //
  //   emit.startColor = {1.0f, 0.8f, 0.2f, 1.0f};
  //   emit.endColor = {1.0f, 0.0f, 0.0f, 0.0f};
  //
  //   emit.texture = &m_atlas.GetTexture();
  //   if (auto *r = m_atlas.GetRegion("solidWhite")) {
  //     emit.uvMin = r->uvMin;
  //     emit.uvMax = r->uvMax;
  //   }
  //   emit.layer = engine::RenderLayer::Overlay;
  // }

  // if (ImGui::Button("Save Scene")) {
  //   engine::SceneSerializer::Save(m_scene, m_atlas, "scene.json");
  // }
  //
  // if (ImGui::Button("Load Scene")) {
  //   if (engine::SceneSerializer::Load(m_scene, m_atlas, "scene.json")) {
  //     m_tower = m_scene.FindEntity("Tower");
  //     m_hero = m_scene.FindEntity("Hero");
  //     m_enemy = m_scene.FindEntity("Enemy");
  //     m_coin = m_scene.FindEntity("Coin");
  //
  //     if (m_scene.IsAlive(m_hero)) {
  //       if (auto *idle = m_animations.Get("hero_idle"))
  //         m_scene.AnimatorOf(m_hero).Play(idle);
  //     }
  //
  //     if (m_scene.IsAlive(m_coin)) {
  //       if (auto *coin = m_animations.Get("coin_spin"))
  //         m_scene.AnimatorOf(m_coin).Play(coin);
  //     }
  //   }
  // }

  // ImGui::Separator();
  //
  // if (ImGui::Button("Spawn entity")) {
  //   auto e = m_scene.CreateEntity("Spawned");
  //   auto &t = m_scene.TransformOf(e);
  //   auto &s = m_scene.SpriteOf(e);
  //   t.position = {(float)(rand() % 20 - 10), (float)(rand() % 14 - 7)};
  //   t.scale = {0.5f, 0.5f};
  //   ApplyRegion(e, (rand() & 1) ? "checkerRed" : "checkerBlue");
  //   s.color = {1, 1, 1, 1};
  // }
  ImGui::End();
}
