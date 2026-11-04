#include "engine/graphics/texture/TextureAtlas.hpp"

#include "engine/utils/Logger.hpp"
#include "glad/gl.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <numeric>
#include <stb/stb_image.h>
#include <system_error>
#include <utility>

namespace engine::graphics {

namespace {

std::array<std::string, 3> kAllowedImageExtensions = {".png", ".jpg", ".jpeg"};

int NextPowerOf2(int v) {
  int p = 1;
  while (p < v)
    p <<= 1;
  return p;
}

std::uint8_t *PixelAt(std::vector<std::uint8_t> &data, int stride, int x,
                      int y) {
  return data.data() + (static_cast<std::size_t>(y) * stride + x) * 4;
}

const std::uint8_t *PixelAt(const std::vector<std::uint8_t> &data, int stride,
                            int x, int y) {
  return data.data() + (static_cast<std::size_t>(y) * stride + x) * 4;
}

void Unpack(std::uint32_t c, std::uint8_t out[4]) {
  out[0] = static_cast<std::uint8_t>((c >> 24) & 0xFF); // R
  out[1] = static_cast<std::uint8_t>((c >> 16) & 0xFF); // G
  out[2] = static_cast<std::uint8_t>((c >> 8) & 0xFF);  // B
  out[3] = static_cast<std::uint8_t>((c >> 0) & 0xFF);  // A
}

} // namespace

int TextureAtlas::AddFromDirectory(const std::string &directory) {
  namespace fs = std::filesystem;

  std::error_code ec;
  if (!fs::is_directory(directory, ec) || ec) {
    Logger::Info(
        "[TextureAtlas] no directory at %s - using precedural fallback",
        directory.c_str());
    return 0;
  }

  const fs::path root = fs::weakly_canonical(directory, ec);
  if (ec) {
    Logger::Error("[TextureAtlas] canonical path failed for %s",
                  directory.c_str());
    return 0;
  }

  int added = 0;
  for (const auto &entry : fs::recursive_directory_iterator(directory, ec)) {
    if (ec)
      break;
    if (!entry.is_regular_file())
      continue;

    const auto &path = entry.path();
    auto it = std::find(kAllowedImageExtensions.begin(),
                        kAllowedImageExtensions.end(), path.extension());
    if (it == kAllowedImageExtensions.end())
      continue;

    // Region name = relative path from root, '/' replaced with '_',
    // extension dropped e.g.:
    //   hero/idle_0.png      -> hero_idle_0
    //   hero/idle/0.png      -> hero_idle_0
    //   hero.png             -> hero

    const fs::path rel = fs::relative(path, root, ec);
    if (ec)
      continue;

    std::string name;
    for (const auto &part : rel) {
      if (!name.empty())
        name += "_";
      name += (part.has_extension() ? part.stem().string() : part.string());
    }

    if (AddFromFile(name, path.string())) {
      ++added;
    }
  }

  if (added > 0) {
    Logger::Info("[TextureAtlas] loaded %d Image(s) from %s", added,
                 directory.c_str());
  }

  return added;
}

bool TextureAtlas::AddFromFile(const std::string &name,
                               const std::string &path) {
  stbi_set_flip_vertically_on_load(true);
  int w = 0;
  int h = 0;
  int channels = 0;
  stbi_uc *data = stbi_load(path.c_str(), &w, &h, &channels, STBI_rgb_alpha);
  if (!data) {
    const char *dsc = stbi_failure_reason() ? stbi_failure_reason() : "?";
    Logger::Error("[TextureAtlas] failed to load: %s (%s)", path.c_str(), dsc);
    return false;
  }

  Source src;
  src.name = name;
  src.width = w;
  src.height = h;
  src.rgba8.assign(data, data + static_cast<std::size_t>(w) * h * 4);
  stbi_image_free(data);
  m_sources.push_back(std::move(src));
  return true;
}

bool TextureAtlas::AddFromPixels(const std::string &name, int w, int h,
                                 const std::uint8_t *rgba) {
  if (w <= 0 || h <= 0 || !rgba)
    return false;
  Source src;
  src.name = name;
  src.width = w;
  src.height = h;
  src.rgba8.assign(rgba, rgba + static_cast<std::size_t>(w) * h * 4);
  m_sources.push_back(std::move(src));
  return true;
}

bool TextureAtlas::AddSolid(const std::string &name, std::uint32_t rgba,
                            int size) {
  if (size < 1)
    size = 1;
  std::uint8_t c[4];
  Unpack(rgba, c);
  std::vector<std::uint8_t> pixels(static_cast<std::size_t>(size) * size * 4);
  for (int i = 0; i < size * size; ++i) {
    pixels[i * 4 + 0] = c[0];
    pixels[i * 4 + 1] = c[1];
    pixels[i * 4 + 2] = c[2];
    pixels[i * 4 + 3] = c[3];
  }
  return AddFromPixels(name, size, size, pixels.data());
}

bool TextureAtlas::AddCheckerboard(const std::string &name, int cells,
                                   std::uint32_t a, std::uint32_t b) {
  if (cells < 1)
    cells = 1;
  constexpr int kCellPx = 8;
  const int size = cells * kCellPx;

  std::uint8_t ca[4], cb[4];
  Unpack(a, ca);
  Unpack(b, cb);

  std::vector<std::uint8_t> pixels(static_cast<std::size_t>(size) * size * 4);
  for (int y = 0; y < size; ++y) {
    for (int x = 0; x < size; ++x) {
      const bool even = ((x / kCellPx) + (y / kCellPx)) % 2 == 0;
      const std::uint8_t *src = even ? ca : cb;
      const std::size_t idx = (static_cast<std::size_t>(y) * size + x) * 4;
      pixels[idx + 0] = src[0];
      pixels[idx + 1] = src[1];
      pixels[idx + 2] = src[2];
      pixels[idx + 3] = src[3];
    }
  }
  return AddFromPixels(name, size, size, pixels.data());
}

bool TextureAtlas::Pack() {
  if (m_sources.empty()) {
    Logger::Error("[TextureAtlas] Pack called with no sources");
    return false;
  }

  GLint maxTex = 0;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTex);
  if (maxTex <= 0)
    maxTex = 4096;
  Logger::Info("[TextureAtlas] GL_MAX_TEXTURE_SIZE = %d", maxTex);

  constexpr int kPad = 1;
  const std::size_t n = m_sources.size();

  std::vector<std::size_t> order(n);
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
    return m_sources[a].height > m_sources[b].height;
  });

  std::vector<glm::ivec2> bestPositions;
  int bestSize = 0;

  for (int size = 256; size <= maxTex; size *= 2) {
    std::vector<glm::ivec2> pos(n);
    int shelfY = 0;
    int shelfH = 0;
    int cursorX = 0;
    bool fits = true;

    for (std::size_t idx : order) {
      const auto &s = m_sources[idx];
      const int w = s.width + 2 * kPad;
      const int h = s.height + 2 * kPad;

      if (w > size || h > size) {
        fits = false;
        break;
      }

      if (cursorX + w > size) {
        shelfY += shelfH;
        shelfH = 0;
        cursorX = 0;
      }
      if (shelfY + h > size) {
        fits = false;
        break;
      }

      pos[idx] = {cursorX + kPad, shelfY + kPad};
      cursorX += w;
      shelfH = std::max(shelfH, h);
    }

    if (fits) {
      bestSize = size;
      bestPositions = std::move(pos);
      break;
    }
  }

  if (bestSize == 0) {
    Logger::Error("[TextureAtlas] cannot pack %zu sources within %dx%d", n,
                  maxTex, maxTex);
    return false;
  }

  std::vector<std::uint8_t> dst(
      static_cast<std::size_t>(bestSize) * bestSize * 4, 0);

  m_regions.clear();

  for (std::size_t i = 0; i < n; ++i) {
    const auto &s = m_sources[i];
    const int px = bestPositions[i].x;
    const int py = bestPositions[i].y;

    for (int yy = -kPad; yy < s.height + kPad; ++yy) {
      for (int xx = -kPad; xx < s.width + kPad; ++xx) {
        const int sx = std::clamp(xx, 0, s.width - 1);
        const int sy = std::clamp(yy, 0, s.height - 1);
        const auto *sp = PixelAt(s.rgba8, s.width, sx, sy);
        auto *dp = PixelAt(dst, bestSize, px + xx, py + yy);
        dp[0] = sp[0];
        dp[1] = sp[1];
        dp[2] = sp[2];
        dp[3] = sp[3];
      }
    }

    AtlasRegion r;
    r.uvMin = {static_cast<float>(px) / bestSize,
               static_cast<float>(py) / bestSize};
    r.uvMax = {static_cast<float>(px + s.width) / bestSize,
               static_cast<float>(py + s.height) / bestSize};
    m_regions[s.name] = r;
  }

  m_texture.Upload(bestSize, bestSize, dst.data(), /*generateMipmaps=*/false);

  Logger::Info("[TextureAtlas] packed %zu sources into %d x %d", n, bestSize,
               bestSize);
  return m_texture.IsValid();
}

const AtlasRegion *TextureAtlas::GetRegion(const std::string &name) const {
  auto it = m_regions.find(name);
  return it == m_regions.end() ? nullptr : &it->second;
}

bool TextureAtlas::HasSource(const std::string &name) const {
  for (const auto &s : m_sources) {
    if (s.name == name)
      return true;
  }
  return false;
}
bool TextureAtlas::HasRegion(const std::string &name) const {
  return m_regions.count(name) > 0;
}

std::vector<std::string> TextureAtlas::RegionNames() const {
  std::vector<std::string> out;
  out.reserve(m_regions.size());
  for (const auto &[name, _] : m_regions)
    out.push_back(name);
  std::sort(out.begin(), out.end());
  return out;
}

std::vector<std::string> TextureAtlas::SourceNames() const {
  std::vector<std::string> out;
  out.reserve(m_sources.size());
  for (const auto &s : m_sources)
    out.push_back(s.name);
  std::sort(out.begin(), out.end());
  return out;
}

} // namespace engine::graphics
