#include <array>
#include <cstdint> // ← WAJIB: std::uint8_t
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

struct Ppm {
  int w = 0;
  int h = 0;
  std::vector<std::uint8_t> rgb;
};

bool ReadPpm(const fs::path &p, Ppm &out) {
  std::FILE *f = std::fopen(p.string().c_str(), "rb");
  if (!f)
    return false;

  char magic[3] = {0};
  int w = 0, h = 0, maxv = 0;
  if (std::fscanf(f, "%2s", magic) != 1 || std::strcmp(magic, "P6") != 0 ||
      std::fscanf(f, "%d %d %d", &w, &h, &maxv) != 3 || maxv != 255 || w <= 0 ||
      h <= 0) {
    std::fclose(f);
    return false;
  }
  std::fgetc(f);

  out.w = w;
  out.h = h;
  out.rgb.resize(static_cast<std::size_t>(w) * static_cast<std::size_t>(h) *
                 3u);
  const bool ok =
      std::fread(out.rgb.data(), 1, out.rgb.size(), f) == out.rgb.size();
  std::fclose(f);
  return ok;
}

int MaxChannelDelta(const Ppm &a, const Ppm &b) {
  if (a.w != b.w || a.h != b.h)
    return 255;
  int maxd = 0;
  for (std::size_t i = 0; i < a.rgb.size(); ++i) {
    const int d =
        std::abs(static_cast<int>(a.rgb[i]) - static_cast<int>(b.rgb[i]));
    if (d > maxd)
      maxd = d;
  }
  return maxd;
}

struct Case {
  const char *name;
  const char *scene;
  const char *golden;
};

} // namespace

int main(int argc, char **argv) {
  bool generate = false;
  fs::path goldenMain;
  for (int i = 1; i < argc; ++i) {
    const std::string a = argv[i];
    if (a == "--generate") {
      generate = true;
    } else if (goldenMain.empty()) {
      goldenMain = a;
    }
  }
  if (goldenMain.empty()) {
    std::fprintf(stderr, "usage: test_golden [--generate] <golden_main>\n");
    return 2;
  }

  const fs::path root = fs::path(__FILE__).parent_path();
  const fs::path sceneDir = root / "scenes";
  const fs::path goldenDir = root / "golden";
  const fs::path outDir = fs::temp_directory_path() / "catalyst_golden";

  std::error_code ec;
  fs::create_directories(outDir, ec);

  const std::array<Case, 4> cases = {{
      {"A", "sprites_only.json", "A.ppm"},
      {"B", "lights_only.json", "B.ppm"},
      {"C", "sprites_lights.json", "C.ppm"},
      {"D", "empty.json", "D.ppm"},
  }};

  constexpr int kFrames = 30;
  constexpr int kMaxDelta = 2;

  int failures = 0;
  if (generate)
    fs::create_directories(goldenDir, ec);

  for (const auto &c : cases) {
    const fs::path out = outDir / (std::string(c.name) + ".ppm");
    const fs::path scene = sceneDir / c.scene;

    std::string cmd = "\"" + goldenMain.string() + "\"" + " --frames " +
                      std::to_string(kFrames) + " --screenshot \"" +
                      out.string() + "\" --scene \"" + scene.string() + "\"";

    if (std::system(cmd.c_str()) != 0) {
      std::fprintf(stderr, "FAIL %s: golden_main exited non-zero\n", c.name);
      ++failures;
      continue;
    }
    if (!fs::exists(out)) {
      std::fprintf(stderr, "FAIL %s: no screenshot produced\n", c.name);
      ++failures;
      continue;
    }

    if (generate) {
      const fs::path dst = goldenDir / c.golden;
      fs::copy_file(out, dst, fs::copy_options::overwrite_existing, ec);
      std::printf("gen  %s -> %s\n", c.name, dst.string().c_str());
      continue;
    }

    const fs::path golden = goldenDir / c.golden;
    Ppm actual, expected;
    if (!ReadPpm(out, actual)) {
      std::fprintf(stderr, "FAIL %s: cannot read %s\n", c.name,
                   out.string().c_str());
      ++failures;
      continue;
    }
    if (!ReadPpm(golden, expected)) {
      std::fprintf(stderr,
                   "FAIL %s: cannot read golden %s (run with --generate)\n",
                   c.name, golden.string().c_str());
      ++failures;
      continue;
    }

    const int delta = MaxChannelDelta(actual, expected);
    if (delta > kMaxDelta) {
      std::fprintf(stderr, "FAIL %s: maxChannelDelta=%d (> %d)\n", c.name,
                   delta, kMaxDelta);
      ++failures;
    } else {
      std::printf("ok   %s (maxChannelDelta=%d)\n", c.name, delta);
    }
  }

  if (generate) {
    std::printf("generated %zu golden images in %s\n", cases.size(),
                goldenDir.string().c_str());
    return 0;
  }
  return failures == 0 ? 0 : 1;
}
