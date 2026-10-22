#pragma once

#include "GLFW/glfw3.h"
#include "engine/core/Game.hpp"
#include "engine/graphics/Camera.hpp"
#include <engine/core/Logger.hpp>

#include <glad/gl.h>
#include <imgui.h>

#include <cmath>

class KingdomRushGame final : public engine::Game {
public:
  KingdomRushGame()
      : engine::Game([] {
          engine::WindowConfig cfg;
          cfg.title = "Kingdom Rush Frontiers";
          return cfg;
        }()) {
    BuildDemoScene();
  }

protected:
  void OnInitialize() override {
    engine::Logger::Info("OnInitialize");

    engine::Camera camera;
    camera.SetViewport(1280, 720);
    camera.SetOrthoSize(10.0f); // Half-height = 10 world units
    camera.SetPosition({0, 0, 0});

    auto viewProjection = camera.ViewProjectionMatrix();
    engine::Logger::Info("View Projection %.2f %.2f %.2f %.2f",
                         viewProjection[0][0], viewProjection[1][0],
                         viewProjection[2][0], viewProjection[3][0]);

    auto centre = camera.WorldToScreen({0, 0});
    engine::Logger::Info("World(0, 0) -> Screen(%.2f, %.2f)", centre.x,
                         centre.y);

    // World (17.78, 10) should map to the top-right pixel.
    auto tr = camera.WorldToScreen({10.0f * (16.0f / 9.0f), 10.0f});
    engine::Logger::Info("World(17.78, 10) -> screen(%.2f, %.2f)", tr.x, tr.y);

    // Round-trip: screen -> world -> screen must match.
    auto back = camera.ScreenToWorld({640.0f, 360.0f});
    engine::Logger::Info("Screen(640,360) -> world(%.2f, %.2f)", back.x,
                         back.y);
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
      const float angle = glfwGetTime() * 1.0f;
      t.position = {std::cos(angle) * 6.0f, std::sin(angle) * 4.0f};
      t.rotation = angle;
    }
  }

  void OnRender() override {
    glClearColor(0.15f, 0.20f, 0.30f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
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
    DrawWorldGrid();
    DrawScene();
    DrawCameraControls();
#ifdef DEBUG
    DrawSceneDebug();
#endif // DEBUG
  }

private:
  engine::Entity m_Tower = engine::kInvalidEntity;
  engine::Entity m_Hero = engine::kInvalidEntity;
  engine::Entity m_Enemy = engine::kInvalidEntity;

  void DrawWorldGrid() {
    auto &camera = GetCamera();
    auto *draw = ImGui::GetBackgroundDrawList();

    // Visible world bounds: compute from screen corners.
    const auto tl = camera.ScreenToWorld({0, 0});
    const auto br = camera.ScreenToWorld(
        {(float)camera.ViewportWidth(), (float)camera.ViewportHeight()});

    const float minX = std::min(tl.x, br.x);
    const float maxX = std::max(tl.x, br.x);
    const float minY = std::min(tl.y, br.y);
    const float maxY = std::max(tl.y, br.y);

    const float step = 1.0f;
    const ImU32 lineColor = IM_COL32(80, 90, 100, 200);
    const ImU32 axisColor = IM_COL32(200, 100, 120, 255);

    for (float x = std::floor(minX / step) * step; x <= maxX; x += step) {
      const auto a = camera.WorldToScreen({x, minY});
      const auto b = camera.WorldToScreen({x, maxY});
      draw->AddLine({a.x, a.y}, {b.x, b.y},
                    std::abs(x) < 0.001f ? axisColor : lineColor);
    }

    for (float y = std::floor(minY / step) * step; y <= maxY; y += step) {
      const auto a = camera.WorldToScreen({minX, y});
      const auto b = camera.WorldToScreen({maxX, y});
      draw->AddLine({a.x, a.y}, {b.x, b.y},
                    std::abs(y) < 0.001f ? axisColor : lineColor);
    }

    // Mark world origin
    const auto origin = camera.WorldToScreen({0, 0});
    draw->AddCircleFilled({origin.x, origin.y}, 4.0f,
                          IM_COL32(255, 100, 100, 255));
  }

  void DrawCameraControls() {
    ImGui::Begin("Camera");
    auto &cam = GetCamera();
    auto p = cam.Position();
    ImGui::Text("Pos:  (%.2f, %.2f)", p.x, p.y);
    ImGui::Text("Zoom: %.2f", cam.Zoom());
    ImGui::Text("Rot:  %.2f rad", cam.Rotation());
    ImGui::Text("Ortho half-height: %.2f", cam.OrthoSize());
    ImGui::Separator();
    ImGui::Text("WASD  move");
    ImGui::Text("Q / E rotate");
    ImGui::Text("Wheel zoom at cursor");
    ImGui::End();
  }

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
    scene.SpriteOf(m_Hero).color = {0.3f, 0.9f, 0.3f, 1.0f};

    // Yellow Enemy
    m_Enemy = scene.CreateEntity("Enemy");
    scene.TransformOf(m_Enemy).scale = {0.5f, 0.5f};
    scene.SpriteOf(m_Enemy).color = {0.9f, 0.9f, 0.3f, 1.0f};
  }

  void DrawScene() {
    auto &camera = GetCamera();
    auto *draw = ImGui::GetBackgroundDrawList();

    GetScene().ForEach([&](engine::Entity, const engine::Transform &t,
                           const engine::Sprite &s) {
      const float c = std::cos(t.rotation);
      const float sn = std::sin(t.rotation);
      const glm::vec2 half = t.scale * 0.5f;

      auto rotate = [&](glm::vec2 p) {
        return glm::vec2(p.x * c - p.y * sn, p.x * sn + p.y * c);
      };

      glm::vec2 corners[4] = {
          rotate({-half.x, -half.y}) + t.position,
          rotate({half.x, -half.y}) + t.position,
          rotate({half.x, half.y}) + t.position,
          rotate({-half.x, half.y}) + t.position,
      };

      ImVec2 pts[4];

      for (int i = 0; i < 4; ++i) {
        auto p = camera.WorldToScreen(corners[i]);
        pts[i] = {p.x, p.y};
      }

      const ImU32 col = ImGui::ColorConvertFloat4ToU32(
          ImVec4(s.color.r, s.color.g, s.color.b, s.color.a));

      draw->AddTriangleFilled(pts[0], pts[1], pts[2], col);
      draw->AddTriangleFilled(pts[0], pts[2], pts[3], col);
    });
  }

  void DrawSceneDebug() {
    ImGui::Begin("Scene");
    ImGui::Text("Entities: %zu", GetScene().Size());
    if (ImGui::Button("Spawn entity")) {
      auto e = GetScene().CreateEntity("Spawned");
      auto &t = GetScene().TransformOf(e);
      auto &s = GetScene().SpriteOf(e);
      t.position = {(float)(rand() % 20 - 10), (float)(rand() % 14 - 7)};
      t.scale = {0.5f, 0.5f};
      s.color = {(rand() % 100) / 100.0f, (rand() % 100) / 100.0f,
                 (rand() % 100) / 100.0f, 1.0f};
    }
    if (ImGui::Button("Destroy 'Hero'") && GetScene().IsAlive(m_Hero)) {
      GetScene().DestroyEntity(m_Hero);
    }
    ImGui::End();
  }
};
