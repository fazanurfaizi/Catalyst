#include "engine/core/Game.hpp"
#include "engine/graphics/Camera.hpp"
#include <engine/core/Logger.hpp>

#include <glad/gl.h>

#include <imgui.h>

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

  void OnUpdate(double /*dt*/) override { IsKeyDown(GLFW_KEY_W); }

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
    static float f = 0.0f;
    ImGui::Begin("Debug");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
    ImGui::End();
  }
};

int main(int /*argc*/, char ** /*argv*/) {
  KingdomRushGame game;
  game.Run();
  return 0;
  // engine::Logger::Info("Booting");
  //
  // glfwSetErrorCallback(glfw_error_callback);
  // if (!glfwInit()) {
  //   engine::Logger::Error("glfwInit failed");
  //   return 1;
  // }
  //
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //
  // GLFWwindow *window = glfwCreateWindow(
  //     kWindowWidth, kWindowHeight, "Kingdom Rush Frontiers", nullptr,
  //     nullptr);
  // if (!window) {
  //   engine::Logger::Error("glfwCreateWindow failed");
  //   glfwTerminate();
  //   return 1;
  // }
  //
  // glfwMakeContextCurrent(window);
  // glfwSwapInterval(1); // vsync
  //
  // if (!gladLoadGL(glfwGetProcAddress)) {
  //   engine::Logger::Error("gladLoadGLLoader failed");
  //   glfwDestroyWindow(window);
  //   glfwTerminate();
  //   return 1;
  // }
  //
  // engine::Logger::Info(std::string("GL: ") +
  //                      reinterpret_cast<const char
  //                      *>(glGetString(GL_VERSION)));
  //
  // // --- ImGui setup
  // ---------------------------------------------------------
  // IMGUI_CHECKVERSION();
  // ImGui::CreateContext();
  // ImGuiIO &io = ImGui::GetIO();
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  // ImGui::StyleColorsDark();
  //
  // ImGui_ImplGlfw_InitForOpenGL(window, /*install_callbacks=*/true);
  // ImGui_ImplOpenGL3_Init(kGlslVersion);
  //
  // bool showDemoWindow = true;
  // bool showAnotherWindow = false;
  // ImVec4 clearColor(0.45f, 0.55f, 0.60f, 1.00f);
  //
  // engine::Logger::Info("Entering main loop");
  //
  // // --- Main loop
  // ----------------------------------------------------------- while
  // (!glfwWindowShouldClose(window)) {
  //   glfwPollEvents();
  //
  //   // Minimised window: don't burn CPU rendering nothing.
  //   if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
  //     glfwWaitEventsTimeout(0.01);
  //     continue;
  //   }
  //
  //   // Start ImGui frame
  //   ImGui_ImplOpenGL3_NewFrame();
  //   ImGui_ImplGlfw_NewFrame();
  //   ImGui::NewFrame();
  //
  //   // Demo window (ships with ImGui — great reference)
  //   if (showDemoWindow) {
  //     ImGui::ShowDemoWindow(&showDemoWindow);
  //   }
  //
  //   // Custom window
  //   {
  //     static float f = 0.0f;
  //     static int counter = 0;
  //
  //     ImGui::Begin("Hello, world!");
  //     ImGui::Text("This is some useful text.");
  //     ImGui::Checkbox("Demo Window", &showDemoWindow);
  //     ImGui::Checkbox("Another Window", &showAnotherWindow);
  //     ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
  //     ImGui::ColorEdit3("clear color", reinterpret_cast<float
  //     *>(&clearColor));
  //
  //     if (ImGui::Button("Button"))
  //       ++counter;
  //     ImGui::SameLine();
  //     ImGui::Text("counter = %d", counter);
  //
  //     ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
  //                 1000.0f / io.Framerate, io.Framerate);
  //     ImGui::End();
  //   }
  //
  //   if (showAnotherWindow) {
  //     ImGui::Begin("Another Window", &showAnotherWindow);
  //     ImGui::Text("Hello from another window!");
  //     if (ImGui::Button("Close Me"))
  //       showAnotherWindow = false;
  //     ImGui::End();
  //   }
  //
  //   // Render
  //   ImGui::Render();
  //
  //   int displayW = 0, displayH = 0;
  //   glfwGetFramebufferSize(window, &displayW, &displayH);
  //   glViewport(0, 0, displayW, displayH);
  //   glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w,
  //                clearColor.z * clearColor.w, clearColor.w);
  //   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //
  //   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  //   glfwSwapBuffers(window);
  // }
  //
  // // --- Shutdown
  // ------------------------------------------------------------
  // ImGui_ImplOpenGL3_Shutdown();
  // ImGui_ImplGlfw_Shutdown();
  // ImGui::DestroyContext();
  //
  // glfwDestroyWindow(window);
  // glfwTerminate();
  //
  // engine::Logger::Info("Shutdown");
  // return 0;
}
