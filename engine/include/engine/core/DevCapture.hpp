#pragma once

#include <filesystem>
namespace engine::core {

struct DevCaptureOptions {
  int frames = -1;
  std::filesystem::path screenshot;
  std::filesystem::path scene;

  bool IsCaptureMode() const { return !screenshot.empty(); }
  bool WantsFixesScene() const { return !scene.empty(); }
};

bool ParseDevCaptureArgs(int argc, char **argv, DevCaptureOptions &out);

bool WritePpmRgba(const std::filesystem::path &path, const unsigned char *rgba,
                  int w, int h);

} // namespace engine::core
