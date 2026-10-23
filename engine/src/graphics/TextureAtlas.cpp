#include "engine/graphics/TextureAtlas.hpp"

#include "engine/core/Logger.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <stb/stb_image.h>
#include <system_error>
#include <utility>

namespace engine {

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

  int added = 0;
  for (const auto &entry : fs::directory_iterator(directory, ec)) {
    if (ec)
      break;
    if (!entry.is_regular_file())
      continue;

    const auto &path = entry.path();
    auto it = std::find(kAllowedImageExtensions.begin(),
                        kAllowedImageExtensions.end(), path.extension());
    if (it == kAllowedImageExtensions.end())
      continue;

    const std::string name = path.stem().string();
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
    Logger::Error("[Atlas] failed to load: %s (%s)", path.c_str(), dsc);
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

  int maxDim = 1;
  for (const auto &s : m_sources) {
    maxDim = std::max(maxDim, std::max(s.width, s.height));
  }
  const int cellSize = NextPowerOf2(maxDim);

  const int n = static_cast<int>(m_sources.size());
  const int gridSize =
      static_cast<int>(std::ceil(std::sqrt(static_cast<float>(n))));
  const int atlasSize = gridSize * cellSize;

  std::vector<std::uint8_t> dst(
      static_cast<std::size_t>(atlasSize) * atlasSize * 4, 0);

  m_regions.clear();

  for (int i = 0; i < n; ++i) {
    const auto &src = m_sources[i];
    const int cellX = i % gridSize;
    const int cellY = i / gridSize;

    const int offX = (cellSize - src.width) / 2;
    const int offY = (cellSize - src.height) / 2;

    for (int y = 0; y < cellSize; ++y) {
      for (int x = 0; x < cellSize; ++x) {
        const int sx = std::clamp(x - offX, 0, src.width - 1);
        const int sy = std::clamp(y - offY, 0, src.height - 1);
        const std::uint8_t *sp = PixelAt(src.rgba8, src.width, sx, sy);
        std::uint8_t *dp =
            PixelAt(dst, atlasSize, cellX * cellSize + x, cellY * cellSize + y);
        dp[0] = sp[0];
        dp[1] = sp[1];
        dp[2] = sp[2];
        dp[3] = sp[3];
      }
    }

    AtlasRegion region;
    region.uvMin.x = static_cast<float>(cellX * cellSize + offX) / atlasSize;
    region.uvMin.y = static_cast<float>(cellY * cellSize + offY) / atlasSize;
    region.uvMax.x =
        static_cast<float>(cellX * cellSize + offX + src.width) / atlasSize;
    region.uvMax.y =
        static_cast<float>(cellY * cellSize + offY + src.height) / atlasSize;
    m_regions[src.name] = region;
  }

  m_texture.Upload(atlasSize, atlasSize, dst.data(), /*generateMipmaps=*/false);

  Logger::Info("[Atlas] packed %d sources into %d x %d", n, atlasSize,
               atlasSize);
  return m_texture.IsValid();
}

const AtlasRegion *TextureAtlas::GetRegion(const std::string &name) const {
  auto it = m_regions.find(name);
  return it == m_regions.end() ? nullptr : &it->second;
}

std::vector<std::string> TextureAtlas::RegionNames() const {
  std::vector<std::string> out;
  out.reserve(m_regions.size());
  for (const auto &[name, _] : m_regions)
    out.push_back(name);
  std::sort(out.begin(), out.end());
  return out;
}

} // namespace engine
