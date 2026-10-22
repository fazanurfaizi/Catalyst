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
        }()) {}

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
    DrawCameraControls();
  }

private:
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
};
