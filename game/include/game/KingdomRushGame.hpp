#pragma once

#include "GLFW/glfw3.h"
#include "engine/core/Game.hpp"
#include "engine/graphics/Camera.hpp"
#include "engine/graphics/SpriteBatch.hpp"
#include "engine/graphics/Texture.hpp"
#include <engine/core/Logger.hpp>

#include <glad/gl.h>
#include <imgui.h>

#include <cmath>
#include <memory>

namespace {
void DrawWorldGrid(engine::SpriteBatch &batch, const engine::Camera &camera) {
  // Visible world bounds: compute from screen corners.
  const auto tl = camera.ScreenToWorld({0.0f, 0.0f});
  const auto br =
      camera.ScreenToWorld({static_cast<float>(camera.ViewportWidth()),
                            static_cast<float>(camera.ViewportHeight())});
  const float minX = std::min(tl.x, br.x);
  const float maxX = std::max(tl.x, br.x);
  const float minY = std::min(tl.y, br.y);
  const float maxY = std::max(tl.y, br.y);

  // --- cell indices covering the visible region ---------------------------
  const int x0 = static_cast<int>(std::floor(minX));
  const int x1 = static_cast<int>(std::ceil(maxX));
  const int y0 = static_cast<int>(std::floor(minY));
  const int y1 = static_cast<int>(std::ceil(maxY));

  // --- guardrail: don't spam quads when zoomed way out --------------------
  constexpr int kMaxCells = 4096;
  const int cols = x1 - x0;
  const int rows = y1 - y0;
  if (cols * rows > kMaxCells)
    return;

  // --- palette ------------------------------------------------------------
  const glm::vec4 evenCell{0.13f, 0.17f, 0.24f, 1.0f};
  const glm::vec4 oddCell{0.16f, 0.20f, 0.28f, 1.0f};
  const glm::vec4 axisCell{0.20f, 0.24f, 0.34f, 1.0f};

  engine::Transform t{};
  engine::Sprite s{};
  t.scale = {1.0f, 1.0f};

  for (int y = y0; y < y1; ++y) {
    for (int x = x0; x < x1; ++x) {
      t.position = {static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f};

      if (x == 0 || y == 0) {
        s.color = axisCell; // axis row/col
      } else {
        s.color = ((x + y) & 1) ? oddCell : evenCell; // checkerboard
      }
      batch.DrawQuad(t, s);
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
    m_Batch = std::make_unique<engine::SpriteBatch>(4096);

    m_checkerRed.CreateCheckerboard(8, 0xCC3333FF, 0x661111FF);
    m_checkerBlue.CreateCheckerboard(8, 0x3366CCFF, 0x112244FF);

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

    if (GetScene().IsAlive(m_Enemy)) {
      auto &t = GetScene().TransformOf(m_Enemy);
      double mx, my;
      glfwGetCursorPos(Window(), &mx, &my);
      const auto world = camera.ScreenToWorld(
          {static_cast<float>(mx), static_cast<float>(my)});
      GetScene().TransformOf(m_Hero).position = world;
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
    glClearColor(0.15f, 0.20f, 0.30f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    auto &camera = GetCamera();
    m_Batch->Begin(camera);

    DrawWorldGrid(*m_Batch, camera);

    GetScene().ForEach([&](engine::Entity e, const engine::Transform &t,
                           const engine::Sprite &s) {
      if (e == m_Hero)
        return;
      m_Batch->DrawQuad(t, s);
    });

    if (GetScene().IsAlive(m_Hero)) {
      m_Batch->DrawQuad(GetScene().TransformOf(m_Hero),
                        GetScene().SpriteOf(m_Hero));
    }

    m_Batch->End();
  }

  void OnShutdown() override {
    m_Batch.reset();
    engine::Logger::Info("OnShutdown");
  }

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
      engine::Logger::Info("Left click");
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
  std::unique_ptr<engine::SpriteBatch> m_Batch;
  engine::Texture m_checkerRed;
  engine::Texture m_checkerBlue;

  engine::Entity m_Tower = engine::kInvalidEntity;
  engine::Entity m_Hero = engine::kInvalidEntity;
  engine::Entity m_Enemy = engine::kInvalidEntity;

  void BuildDemoScene() {
    auto &scene = GetScene();

    // Red Tower
    m_Tower = scene.CreateEntity("Tower");
    scene.TransformOf(m_Tower).position = {-4.0f, -2.0f};
    scene.TransformOf(m_Tower).scale = {1.5f, 1.5f};
    scene.SpriteOf(m_Tower).color = {0.9f, 0.3f, 0.3f, 1.0f};

    // Green Hero
    m_Hero = scene.CreateEntity("Hero");
    scene.TransformOf(m_Hero).scale = {0.7f, 0.7f};
    scene.SpriteOf(m_Hero).color = {1.0f, 1.0f, 1.0f, 1.0f};
    scene.SpriteOf(m_Hero).texture = &m_checkerRed;

    // Yellow Enemy
    m_Enemy = scene.CreateEntity("Enemy");
    scene.TransformOf(m_Enemy).scale = {0.5f, 0.5f};
    scene.SpriteOf(m_Enemy).color = {0.9f, 0.9f, 0.3f, 1.0f};
    scene.SpriteOf(m_Enemy).texture = &m_checkerBlue;
  }

  void DrawSceneDebug() {
    ImGui::Begin("Renderer");
    ImGui::Text("Draw calls: %u", m_Batch->LastDrawCalls());
    ImGui::Text("Quads:      %u", m_Batch->LastQuadCount());
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

    if (ImGui::Button("Spawn entity")) {
      auto e = GetScene().CreateEntity("Spawned");
      auto &t = GetScene().TransformOf(e);
      auto &s = GetScene().SpriteOf(e);
      t.position = {(float)(rand() % 20 - 10), (float)(rand() % 14 - 7)};
      t.scale = {0.5f, 0.5f};
      s.color = {(rand() % 100) / 100.0f, (rand() % 100) / 100.0f,
                 (rand() % 100) / 100.0f, 1.0f};
    }
    ImGui::End();
  }
};
