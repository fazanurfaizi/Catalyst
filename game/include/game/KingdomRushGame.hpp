#pragma once

#include "GLFW/glfw3.h"
#include "engine/animation/AnimationLibrary.hpp"
#include "engine/assets/AssetPath.hpp"
#include "engine/core/Game.hpp"
#include "engine/graphics/Camera.hpp"
#include "engine/graphics/Renderer.hpp"
#include "engine/graphics/TextureAtlas.hpp"
#include "engine/scene/Scene.hpp"
#include <engine/core/Logger.hpp>

#include <glad/gl.h>
#include <imgui.h>

#include <cmath>
#include <string>

namespace {
void DrawWorldGrid(engine::Renderer &renderer, const engine::Camera &camera,
                   const engine::TextureAtlas &atlas) {
  auto *region = atlas.GetRegion("solidWhite");
  if (!region)
    return;

  // Visible world bounds: compute from screen corners.
  const auto tl = camera.ScreenToWorld({0.0f, 0.0f});
  const auto br =
      camera.ScreenToWorld({static_cast<float>(camera.ViewportWidth()),
                            static_cast<float>(camera.ViewportHeight())});
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

  const glm::vec4 evenCell{0.13f, 0.17f, 0.24f, 1.0f};
  const glm::vec4 oddCell{0.16f, 0.20f, 0.28f, 1.0f};
  const glm::vec4 axisCell{0.20f, 0.24f, 0.34f, 1.0f};

  engine::Transform t{};
  engine::Sprite s{};
  s.texture = &atlas.GetTexture();
  s.uvMin = region->uvMin;
  s.uvMax = region->uvMax;
  t.scale = {1.0f, 1.0f};

  for (int y = y0; y < y1; ++y) {
    for (int x = x0; x < x1; ++x) {
      t.position = {static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f};

      if (x == 0 || y == 0) {
        s.color = axisCell; // axis row/col
      } else {
        s.color = ((x + y) & 1) ? oddCell : evenCell; // checkerboard
      }
      renderer.Draw(t, s);
    }
  }
}
} // namespace

class KingdomRushGame final : public engine::Game {
public:
  KingdomRushGame()
      : engine::Game([] {
          engine::WindowConfig cfg;
          cfg.title = "Kingdom Rush Frontiers";
          return cfg;
        }()) {}

protected:
  void OnInitialize() override {
    engine::Logger::Info("OnInitialize");
    SetClearColor({0.15f, 0.20f, 0.30f, 1.0f});

    const int loaded = m_Atlas.AddFromDirectory(engine::AssetPath("sprites"));
    for (const auto &n : m_Atlas.SourceNames())
      engine::Logger::Info("[Game] loaded source: %s", n.c_str());

    m_Atlas.AddSolid("solidWhite", 0xFFFFFFFF, 16);

    if (!m_Atlas.HasRegion("checkerRed"))
      m_Atlas.AddCheckerboard("checkerRed", 8, 0xCC3333FF, 0x661111FF);
    if (!m_Atlas.HasRegion("checkerBlue"))
      m_Atlas.AddCheckerboard("checkerBlue", 8, 0x3366CCFF, 0x112244FF);
    if (!m_Atlas.HasSource("tower"))
      m_Atlas.AddSolid("tower", 0xE65555FF, 16);

    if (!m_Atlas.HasSource("hero_idle_0"))
      for (int i = 0; i < 9; ++i)
        m_Atlas.AddCheckerboard("hero_idle_" + std::to_string(i), 9 + i,
                                0xCC3333FF, 0x661111FF);

    if (!m_Atlas.HasSource("hero_attack_0"))
      for (int i = 0; i < 9; ++i)
        m_Atlas.AddCheckerboard("hero_attack_" + std::to_string(i), 9 + i,
                                0xFFDD66FF, 0xAA8800FF);

    if (!m_Atlas.HasSource("coin_0")) {
      for (int i = 0; i < 6; ++i) {
        m_Atlas.AddCheckerboard("coin_" + std::to_string(i), 4 + 1, 0xFFDD66FF,
                                0xAA8800FF);
      }
    }

    if (!m_Atlas.Pack()) {
      engine::Logger::Error("Atlas pack failed");
    }

    m_Animations.AddStrip("coin_spin", m_Atlas, "coin", 6, 0.2f,
                          /*loop=*/true);

    m_Animations.AddStrip("hero_idle", m_Atlas, "hero_idle", 10, 0.15f, true);
    m_Animations.AddStrip("hero_attack", m_Atlas, "hero_attack", 10, 0.08f,
                          false);
    m_Animations.AddStrip("coin_spin", m_Atlas, "coin", 6, 0.08f, true);

    engine::Logger::Info(
        loaded > 0
            ? "[Game] using %d real PNGs + fallbacks"
            : "[Game] no assets/sprites/*.png — using procedural fallbacks",
        loaded);

    BuildDemoScene();
  }

  void OnUpdate(double dt) override {
    auto &camera = GetCamera();
    const float speed = 5.0f * static_cast<float>(dt);

    if (IsKeyDown(GLFW_KEY_W))
      camera.Move({0.0f, speed, 0.0f});
    if (IsKeyDown(GLFW_KEY_S))
      camera.Move({0.0f, -speed, 0.0f});
    if (IsKeyDown(GLFW_KEY_A))
      camera.Move({-speed, 0.0f, 0.0f});
    if (IsKeyDown(GLFW_KEY_D))
      camera.Move({speed, 0.0f, 0.0f});
    if (IsKeyDown(GLFW_KEY_Q))
      camera.RotateBy(+0.5f * static_cast<float>(dt));
    if (IsKeyDown(GLFW_KEY_E))
      camera.RotateBy(-0.5f * static_cast<float>(dt));

    if (IsKeyDown(GLFW_KEY_ESCAPE))
      glfwSetWindowShouldClose(Window(), GLFW_TRUE);

    if (GetScene().IsAlive(m_Hero)) {
      double mx, my;
      glfwGetCursorPos(Window(), &mx, &my);
      const auto world = camera.ScreenToWorld(
          {static_cast<float>(mx), static_cast<float>(my)});
      GetScene().TransformOf(m_Hero).position = world;

      auto &a = GetScene().AnimatorOf(m_Hero);
      auto *idleClip = m_Animations.Get("hero_idle");
      auto *attackClip = m_Animations.Get("hero_attack");

      if (idleClip && attackClip && a.finished && a.clip == attackClip) {
        a.Play(idleClip);
      }
    }

    // Enemy orbits origin.
    if (GetScene().IsAlive(m_Enemy)) {
      const float angle = static_cast<float>(glfwGetTime());
      auto &t = GetScene().TransformOf(m_Enemy);
      t.position = {std::cos(angle) * 6.0f, std::sin(angle) * 4.0f};
      t.rotation = angle;
    }
  }

  void OnRender() override {
    auto &renderer = GetRenderer();
    auto &camera = GetCamera();

    SetClearColor({0.10f, 0.12f, 0.16f, 1.0f});
    DrawWorldGrid(renderer, camera, m_Atlas);
    renderer.DrawScene(GetScene());
  }

  void OnShutdown() override { engine::Logger::Info("OnShutdown"); }

  void OnKey(int key, int scancode, int action, int mods) override {
    (void)scancode;
    (void)mods;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      glfwSetWindowShouldClose(Window(), GLFW_TRUE);
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
      engine::Logger::Info("Space tapped");
    }
  }

  void OnScroll(double dx, double dy) override {
    (void)dx;
    // Zoom around the cursor for natural feel.
    auto &camera = GetCamera();
    double mx, my;
    glfwGetCursorPos(Window(), &mx, &my);
    const auto before =
        camera.ScreenToWorld({static_cast<float>(mx), static_cast<float>(my)});
    camera.ZoomBy(dy > 0 ? 1.1f : 1.0f / 1.1f);
    const auto after =
        camera.ScreenToWorld({static_cast<float>(mx), static_cast<float>(my)});
    const auto delta = before - after;
    camera.Move({delta.x, delta.y, 0.0f});
  }

  void OnMouseButton(int button, int action, int mods) override {
    (void)mods;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
      if (!GetScene().IsAlive(m_Hero))
        return;

      auto &animation = GetScene().AnimatorOf(m_Hero);
      auto *idleClip = m_Animations.Get("hero_idle");
      auto *attackClip = m_Animations.Get("hero_attack");
      if (!idleClip || !attackClip)
        return;

      if (animation.clip != idleClip)
        return;

      animation.Play(attackClip);
    }
  }

  void OnMouseMove(double x, double y) override {
    (void)x;
    (void)y;
  }

  void OnResize(int w, int h) override {
    engine::Logger::Info("Resized to %d x %d", w, h);
  }

  void OnImGui() override {
#ifndef DEBUG
    DrawSceneDebug();
#endif // DEBUG
  }

private:
  engine::TextureAtlas m_Atlas;
  engine::AnimationLibrary m_Animations;

  engine::Entity m_Coin = engine::kInvalidEntity;
  engine::Entity m_Tower = engine::kInvalidEntity;
  engine::Entity m_Hero = engine::kInvalidEntity;
  engine::Entity m_Enemy = engine::kInvalidEntity;

  void ApplyRegion(engine::Entity e, const char *region) {
    auto *r = m_Atlas.GetRegion(region);
    if (!r) {
      engine::Logger::Warn("[Game] unknown region: %s", region);
      return;
    }
    auto &s = GetScene().SpriteOf(e);
    s.texture = &m_Atlas.GetTexture();
    s.uvMin = r->uvMin;
    s.uvMax = r->uvMax;
  }

  void BuildDemoScene() {
    auto &scene = GetScene();

    // Red Tower
    m_Tower = scene.CreateEntity("Tower");
    scene.TransformOf(m_Tower).position = {-4.0f, -2.0f};
    scene.TransformOf(m_Tower).scale = {1.5f, 1.5f};
    scene.SpriteOf(m_Tower).layer = engine::RenderLayer::Entities;
    ApplyRegion(m_Tower, "tower");

    // Green Hero
    m_Hero = scene.CreateEntity("Hero");
    scene.TransformOf(m_Hero).scale = {1.2f, 1.7f};
    scene.SpriteOf(m_Hero).layer = engine::RenderLayer::Entities;
    scene.SpriteOf(m_Hero).texture = &m_Atlas.GetTexture();

    // Yellow Enemy
    m_Enemy = scene.CreateEntity("Enemy");
    scene.TransformOf(m_Enemy).scale = {0.9f, 1.0f};
    scene.SpriteOf(m_Enemy).layer = engine::RenderLayer::Entities;
    ApplyRegion(m_Enemy, "enemy");

    m_Coin = scene.CreateEntity("Coin");
    scene.TransformOf(m_Coin).position = {3.0f, 3.0f};
    scene.TransformOf(m_Coin).scale = {0.8f, 0.8f};
    scene.SpriteOf(m_Coin).color = {1, 1, 1, 1};
    scene.SpriteOf(m_Coin).texture = &m_Atlas.GetTexture();
    scene.SpriteOf(m_Coin).layer = engine::RenderLayer::Ground;

    if (auto *idleClip = m_Animations.Get("hero_idle")) {
      scene.AnimatorOf(m_Hero).Play(idleClip);
    }

    auto *coinClip = m_Animations.Get("coin_spin");
    if (coinClip)
      scene.AnimatorOf(m_Coin).Play(coinClip);
  }

  void DrawSceneDebug() {
    ImGui::Begin("Renderer");
    ImGui::Text("Draw calls: %u", GetRenderer().DrawCalls());
    ImGui::Text("Quads:      %u", GetRenderer().QuadCount());
    ImGui::Text("Culled:     %u", GetRenderer().CulledCount());
    ImGui::Text("Alive:      %zu", GetScene().Size());
    ImGui::Text("Clips: %d", m_Animations.Count());

    bool cullOn = GetRenderer().CullingEnabled();
    if (ImGui::Checkbox("Culling", &cullOn)) {
      GetRenderer().SetCullingEnabled(cullOn);
    }

    if (GetScene().IsAlive(m_Coin)) {
      auto &a = GetScene().AnimatorOf(m_Coin);
      ImGui::Text("Coin frame: %d / %d", a.currentFrame,
                  a.clip ? a.clip->FrameCount() : 0);
    }
    ImGui::Separator();

    ImGui::Text("Atlas packed: %d sources", m_Atlas.SourceCount());
    ImGui::Text("Regions:");
    for (const auto &name : m_Atlas.RegionNames()) {
      ImGui::BulletText("%s", name.c_str());
    }
    ImGui::Separator();

    auto &cam = GetCamera();
    auto p = cam.Position();
    ImGui::Text("Cam pos: (%.2f, %.2f)", p.x, p.y);
    ImGui::Text("Cam zoom: %.2f", cam.Zoom());
    ImGui::Text("Rot:  %.2f rad", cam.Rotation());
    ImGui::Text("Ortho half-height: %.2f", cam.OrthoSize());
    ImGui::Separator();
    ImGui::Text("WASD  move");
    ImGui::Text("Q / E rotate");
    ImGui::Text("Wheel zoom at cursor");

    if (GetScene().IsAlive(m_Hero)) {
      auto &a = GetScene().AnimatorOf(m_Hero);
      const bool isAttack = a.clip && a.clip == m_Animations.Get("hero_attack");
      ImGui::Text("Hero: %s  frame %d/%d%s", isAttack ? "attack" : "idle",
                  a.currentFrame, a.clip ? a.clip->FrameCount() : 0,
                  a.finished ? " (done)" : "");
    }

    if (ImGui::Button("Spawn entity")) {
      auto e = GetScene().CreateEntity("Spawned");
      auto &t = GetScene().TransformOf(e);
      auto &s = GetScene().SpriteOf(e);
      t.position = {(float)(rand() % 20 - 10), (float)(rand() % 14 - 7)};
      t.scale = {0.5f, 0.5f};
      ApplyRegion(e, (rand() & 1) ? "checkerRed" : "checkerBlue");
      s.color = {1, 1, 1, 1};
    }
    ImGui::End();
  }
};
