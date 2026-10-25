#include "engine/graphics/Font.hpp"

#include "engine/core/Logger.hpp"
#include "glad/gl.h"

#include <fstream>
#include <stb/stb_truetype.h>
#include <vector>

namespace engine {

namespace {
constexpr int kAtlasW = 1024;
constexpr int kAtlasH = 1024;
constexpr uint32_t kCodePointOfFirstChar = 32;
constexpr uint32_t kCharsToIncludeInFontAtlas = 95;
} // namespace

bool Font::LoadFromFile(const std::string &path, float fontSize) {
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    Logger::Error("[Font] Failed to open %s", path.c_str());
    return false;
  }

  // Find the size of the file to allocate memory dynamically
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  // Allocate the buffer
  std::vector<uint8_t> fontDataBuf(static_cast<std::size_t>(size));
  if (!file.read(reinterpret_cast<char *>(fontDataBuf.data()), size)) {
    Logger::Error("[Font] Failed to read %s", path.c_str());
    return false;
  }

  int32_t fontCount = stbtt_GetNumberOfFonts(fontDataBuf.data());
  if (fontCount == -1) {
    Logger::Error("[Font] File doesn't correspond to valid font data");
    return false;
  } else {
    Logger::Info("[Font] file %s contains %d font(s)", path.c_str(), fontCount);
  }

  std::vector<uint8_t> fontAtlasBitmap(kAtlasW * kAtlasH);

  stbtt_packedchar packedChars[kCharsToIncludeInFontAtlas];
  stbtt_pack_context packContext;

  if (!stbtt_PackBegin(&packContext, (unsigned char *)fontAtlasBitmap.data(),
                       kAtlasW, kAtlasH, 0, 1, nullptr)) {
    Logger::Error("[Font] Failed to begin packing context");
    return false;
  }

  stbtt_PackFontRange(&packContext, fontDataBuf.data(), 0, fontSize,
                      kCodePointOfFirstChar, kCharsToIncludeInFontAtlas,
                      packedChars);
  stbtt_PackEnd(&packContext);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glGenTextures(1, &m_textureId);
  glBindTexture(GL_TEXTURE_2D, m_textureId);

  // Upload the data to the GPU
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, kAtlasW, kAtlasH, 0, GL_RED,
               GL_UNSIGNED_BYTE, fontAtlasBitmap.data());

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // Unbind the texture
  glBindTexture(GL_TEXTURE_2D, 0);

  // Extract metrics and UVs safely using the correct constant bound
  for (int i = 0; i < kCharsToIncludeInFontAtlas; ++i) {
    Character ch;
    ch.textureId = m_textureId;
    ch.size = {
        packedChars[i].x1 - packedChars[i].x0,
        packedChars[i].y1 - packedChars[i].y0,
    };
    ch.bearing = {
        static_cast<int>(packedChars[i].xoff),
        static_cast<int>(packedChars[i].yoff),
    };
    ch.advance = packedChars[i].xadvance;
    ch.uvMin = {
        static_cast<float>(packedChars[i].x0) / kAtlasW,
        static_cast<float>(packedChars[i].y1) / kAtlasH,
    };
    ch.uvMax = {
        static_cast<float>(packedChars[i].x1) / kAtlasW,
        static_cast<float>(packedChars[i].y0) / kAtlasH,
    };

    m_characters[32 + i] = ch;
  }

  return true;
}

const Character &Font::GetCharacter(char c) const { return m_characters.at(c); }

} // namespace engine
