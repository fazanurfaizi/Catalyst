#include "engine/asset/importer/texture-atlas/TextureAtlasBuilder.hpp"
#include "engine/utils/Logger.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <filesystem>
#include <numeric>
#include <stb/stb_image.h>
#include <stb/stb_image_resize.h>
#include <vector>

namespace engine::asset {

namespace {

std::array<std::string, 1> kAllowedImageExtensions = {".png"};

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

int TextureAtlasBuilder::AddFromDirectory(const std::string &directory,
                                          int maxDimension) {
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

    if (AddFromFile(name, path.string(), maxDimension)) {
      ++added;
    }
  }

  if (added > 0) {
    Logger::Info("[TextureAtlas] loaded %d Image(s) from %s", added,
                 directory.c_str());
  }

  return added;
}

bool TextureAtlasBuilder::AddFromFile(const std::string &name,
                                      const std::string &path,
                                      int maxDimension) {
  if (HasSource(name)) {
    Logger::Error("[TextureAtlas] duplicate source name: %s", name.c_str());
    return false;
  }

  stbi_set_flip_vertically_on_load_thread(true);
  int w = 0;
  int h = 0;
  int channels = 0;

  stbi_uc *data = stbi_load(path.c_str(), &w, &h, &channels, STBI_rgb_alpha);
  stbi_set_flip_vertically_on_load_thread(false);

  if (!data) {
    const char *dsc = stbi_failure_reason() ? stbi_failure_reason() : "?";
    Logger::Error("[TextureAtlas] failed to load: %s (%s)", path.c_str(), dsc);
    return false;
  }

  Source src;
  src.name = name;

  if (maxDimension > 0 && (w > maxDimension || h > maxDimension)) {
    const int newW = (w >= h) ? maxDimension : (w * maxDimension / h);
    const int newH = (h >= w) ? maxDimension : (h * maxDimension / w);

    if (newW <= 0 || newH <= 0) {
      stbi_image_free(data);
      Logger::Error("[TextureAtlas] invalid resize target for: %s",
                    path.c_str());
      return false;
    }

    src.width = newW;
    src.height = newH;
    src.rgba8.resize(static_cast<std::size_t>(newW) * newH * 4);

    if (!stbir_resize_uint8(data, w, h, 0, src.rgba8.data(), newW, newH, 0,
                            STBI_rgb_alpha)) {
      stbi_image_free(data);
      Logger::Error("[TextureAtlas] resize failed for: %s", path.c_str());
      return false;
    }
  } else {
    src.width = w;
    src.height = h;
    src.rgba8.assign(data, data + static_cast<std::size_t>(w) * h * 4);
  }

  stbi_image_free(data);

  AddSource(std::move(src));

  return true;
}

bool TextureAtlasBuilder::AddFromPixels(const std::string &name, int w, int h,
                                        const std::uint8_t *rgba8) {
  if (w <= 0 || h <= 0 || !rgba8)
    return false;

  if (HasSource(name)) {
    Logger::Error("[TextureAtlas] duplicate source name: %s", name.c_str());
    return false;
  }

  Source src;
  src.name = name;
  src.width = w;
  src.height = h;
  src.rgba8.assign(rgba8, rgba8 + static_cast<std::size_t>(w) * h * 4);

  AddSource(std::move(src));

  return true;
}

void TextureAtlasBuilder::AddSource(Source &&src) {
  m_sourceNames.emplace(src.name);
  m_sources.push_back(std::move(src));
}

bool TextureAtlasBuilder::AddSolid(const std::string &name, std::uint32_t rgba,
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

bool TextureAtlasBuilder::AddCheckerboard(const std::string &name, int cells,
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

bool TextureAtlasBuilder::Pack(Packed &out, int maxSize) const {
  if (m_sources.empty()) {
    Logger::Error("[TextureAtlasBuilder] Pack called with no sources");
    return false;
  }

  const std::size_t n = m_sources.size();

  int sizeLimit = 256;
  while (sizeLimit * 2 <= maxSize)
    sizeLimit *= 2;

  constexpr int kPad = 1;

  std::vector<std::size_t> order(n);
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
    return m_sources[a].height > m_sources[b].height;
  });

  std::vector<glm::ivec2> bestPositions;
  int bestSize = 0;

  for (int size = 256; size <= sizeLimit; size *= 2) {
    std::vector<glm::ivec2> pos(n);
    int shelfY = 0, shelfH = 0, cursorX = 0;
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
    int largestW = 0, largestH = 0;
    std::size_t totalPaddedArea = 0;
    for (const auto &s : m_sources) {
      largestW = std::max(largestW, s.width);
      largestH = std::max(largestH, s.height);
      totalPaddedArea += static_cast<std::size_t>(s.width + 2) * (s.height + 2);
    }
    Logger::Error(
        "[TextureAtlasBuilder] cannot pack %zu sources within %dx%d "
        "(largest source %dx%d, total padded area %zu px = %.1f%% of %d²)",
        n, sizeLimit, sizeLimit, largestW, largestH, totalPaddedArea,
        100.0 * totalPaddedArea / (double(sizeLimit) * sizeLimit), sizeLimit);
    return false;
  }

  out.width = bestSize;
  out.height = bestSize;
  out.rgba8.assign(static_cast<std::size_t>(bestSize) * bestSize * 4, 0);
  out.regions.clear();
  out.regions.reserve(n);

  for (std::size_t i = 0; i < n; ++i) {
    const auto &s = m_sources[i];
    const int px = bestPositions[i].x;
    const int py = bestPositions[i].y;

    for (int yy = -kPad; yy < s.height + kPad; ++yy) {
      for (int xx = -kPad; xx < s.width + kPad; ++xx) {
        const int sx = std::clamp(xx, 0, s.width - 1);
        const int sy = std::clamp(yy, 0, s.height - 1);
        const auto *sp = PixelAt(s.rgba8, s.width, sx, sy);
        auto *dp = PixelAt(out.rgba8, out.width, px + xx, py + yy);
        dp[0] = sp[0];
        dp[1] = sp[1];
        dp[2] = sp[2];
        dp[3] = sp[3];
      }
    }

    resources::AtlasRegion region;
    region.uvMin = {static_cast<float>(px) / bestSize,
                    static_cast<float>(py) / bestSize};
    region.uvMax = {static_cast<float>(px + s.width) / bestSize,
                    static_cast<float>(py + s.height) / bestSize};

    out.regions.emplace_back(s.name, region);
  }

  Logger::Info("[TextureAtlasBuilder] packed %zu sources into %dx%d", n,
               bestSize, bestSize);
  return true;
}

} // namespace engine::asset
