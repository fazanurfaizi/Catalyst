#include "game/KingdomRushGame.hpp"

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
