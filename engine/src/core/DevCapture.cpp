#include "engine/core/DevCapture.hpp"
#include "engine/utils/Logger.hpp"

#include <cstdio>
#include <cstdlib>
#include <string_view>
#include <vector>

namespace engine::core {

bool ParseDevCaptureArgs(int argc, char **argv, DevCaptureOptions &out) {
  for (int i = 1; i < argc; ++i) {
    std::string_view a{argv[i]};
    if (a == "--frames" && i + 1 < argc) {
      out.frames = std::atoi(argv[++i]);
      if (out.frames <= 0) {
        Logger::Error("[DevCapture] --frames must be > 0");
        return false;
      }
    } else if (a == "--screenshot" && i + 1 < argc) {
      out.screenshot = argv[++i];
    } else if (a == "--scene" && i + 1 < argc) {
      out.scene = argv[++i];
    }
    // Unknown flags ignored: game may pass its own.
  }
  if (!out.screenshot.empty() && out.frames < 0) {
    Logger::Error("[DevCapture] --screenshot requires --frames");
    return false;
  }
  return true;
}

bool WritePpmRgba(const std::filesystem::path &path, const std::uint8_t *rgba,
                  int w, int h) {
  if (!rgba || w <= 0 || h <= 0)
    return false;

  std::FILE *f = std::fopen(path.string().c_str(), "wb");
  if (!f) {
    Logger::Error("[DevCapture] cannot open %s for writing",
                  path.string().c_str());
    return false;
  }

  std::fprintf(f, "P6\n%d %d\n255\n", w, h);

  std::vector<std::uint8_t> row(static_cast<std::size_t>(w) * 3u);
  for (int y = h - 1; y >= 0; --y) {
    for (int x = 0; x < w; ++x) {
      const std::size_t idx =
          (static_cast<std::size_t>(y) * static_cast<std::size_t>(w) +
           static_cast<std::size_t>(x)) *
          4u;
      row[static_cast<std::size_t>(x) * 3u + 0] = rgba[idx + 0];
      row[static_cast<std::size_t>(x) * 3u + 1] = rgba[idx + 1];
      row[static_cast<std::size_t>(x) * 3u + 2] = rgba[idx + 2];
    }
    std::fwrite(row.data(), 1, row.size(), f);
  }

  std::fclose(f);
  Logger::Info("[DevCapture] wrote %s", path.string().c_str());
  return true;
}

} // namespace engine::core
