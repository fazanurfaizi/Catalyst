#include "engine/graphics/text/TextLayout.hpp"
#include <sstream>

namespace engine::graphics {

namespace {

// Decode UTF-8
std::vector<std::uint32_t> DecodeUtf8(const std::string &text) {
  std::vector<std::uint32_t> codepoints;
  codepoints.reserve(text.size());

  std::size_t i = 0;
  while (i < text.size()) {
    const unsigned char c = static_cast<unsigned char>(text[i]);
    std::uint32_t cp = 0;
    int extra = 0;

    if (c < 0x80) {
      cp = c;
      extra = 0;
    } else if ((c & 0xE0) == 0xC0) {
      cp = c & 0x1F;
      extra = 1;
    } else if ((c & 0xF0) == 0xE0) {
      cp = c & 0x0F;
      extra = 2;
    } else if ((c & 0xF8) == 0xF0) {
      cp = c & 0x07;
      extra = 3;
    } else {
      i++;
      continue;
    }

    if (i + extra >= text.size())
      break;

    bool valid = true;
    for (int k = 0; k < extra; ++k) {
      const unsigned char cc = static_cast<unsigned char>(text[i + 1 + k]);
      if ((cc & 0xC0) != 0x80) {
        valid = false;
        break;
      }
    }
    if (valid)
      codepoints.push_back(cp);

    i += 1 + extra;
  }

  return codepoints;
}

const Glyph *ResolveGlyph(const Font &font, std::uint32_t cp) {
  if (const Glyph *ch = font.FindGlyph(cp))
    return ch;
  if (const Glyph *ch = font.FindGlyph('?'))
    return ch;
  return font.FindGlyph(' ');
}

} // namespace

float TextLayout::MeasureStringWidth(const Font &font, const std::string &text,
                                     float scale) {
  float width = 0.0f;
  for (std::uint32_t c : DecodeUtf8(text)) {
    if (const Glyph *glyph = ResolveGlyph(font, c)) {
      width += glyph->advance * scale;
    }
  }
  return width;
}

std::vector<LineInfo> TextLayout::WrapText(const Font &font,
                                           const std::string &text,
                                           float maxWidth, float scale) {
  std::vector<LineInfo> lines;

  float spaceWidth = 0.0f;
  if (const Glyph *glyph = font.FindGlyph(' ')) {
    spaceWidth = glyph->advance * scale;
  }

  // Normalize CRLF/CR to a single '\n'
  std::string normalized;
  normalized.reserve(text.size());
  for (std::size_t i = 0; i < text.size(); ++i) {
    if (text[i] == '\r') {
      if (i + 1 < text.size() && text[i + 1] == '\n')
        continue;
      normalized.push_back('\n');
    } else {
      normalized.push_back(text[i]);
    }
  }

  std::size_t start = 0;
  while (start <= normalized.size()) {
    const std::size_t end = normalized.find('\n', start);
    const bool last = (end == std::string::npos);
    const std::string paragraph =
        last ? normalized.substr(start) : normalized.substr(start, end - start);

    std::istringstream wordStream(paragraph);
    std::string word;
    std::string currentLine;
    float currentLineWidth = 0.0f;
    bool anyWords = false;

    while (wordStream >> word) {
      anyWords = true;
      const float wordWidth = MeasureStringWidth(font, word, scale);

      if (currentLineWidth == 0.0f) {
        currentLine = word;
        currentLineWidth = wordWidth;
      } else {
        const float testWidth = currentLineWidth + spaceWidth + wordWidth;
        if (maxWidth > 0.0f && testWidth > maxWidth) {
          lines.push_back({currentLine, currentLineWidth});
          currentLine = word;
          currentLineWidth = wordWidth;
        } else {
          currentLine.push_back(' ');
          currentLine += word;
          currentLineWidth = testWidth;
        }
      }
    }

    // blank line
    lines.push_back(anyWords ? LineInfo{currentLine, currentLineWidth}
                             : LineInfo{"", 0.0f});

    if (last)
      break;
    start = end + 1;
  }

  return lines;
}

LayoutMetrics TextLayout::ComputeLayout(const Font &font,
                                        const std::string &text, float maxWidth,
                                        float scale) {
  LayoutMetrics metrics;
  metrics.lines = WrapText(font, text, maxWidth, scale);
  metrics.lineHeight = font.GetLineHeight() * scale;

  for (const auto &line : metrics.lines) {
    if (line.width > metrics.maxLineWidth)
      metrics.maxLineWidth = line.width;
  }

  metrics.totalHeight =
      metrics.lines.empty()
          ? 0.0f
          : static_cast<float>(metrics.lines.size()) * metrics.lineHeight;

  return metrics;
}

void TextLayout::BuildTextBatch(const Font &font, const std::string &text,
                                float maxWidth, float scale,
                                TextAlignment alignment,
                                std::vector<Vertex> &outVertices,
                                const glm::vec2 &origin) {
  outVertices.clear();

  const LayoutMetrics metrics = ComputeLayout(font, text, maxWidth, scale);

  const float boxWidth = (maxWidth > 0.0f) ? maxWidth : metrics.maxLineWidth;
  const float lineHeight = metrics.lineHeight;

  // 6 vertices per decoded codepoint
  outVertices.reserve(text.size() * 6);

  float cursorY = origin.y;

  for (const auto &line : metrics.lines) {
    float cursorX = origin.x;
    if (alignment == TextAlignment::Center) {
      cursorX += (boxWidth - line.width) * 0.5f;
    } else if (alignment == TextAlignment::Right) {
      cursorX += boxWidth - line.width;
    }

    for (std::uint32_t cp : DecodeUtf8(line.text)) {
      const Glyph *glyph = ResolveGlyph(font, cp);
      if (!glyph)
        continue;

      if (glyph->size.x == 0 || glyph->size.y == 0) {
        cursorX += glyph->advance * scale;
        continue;
      }

      const float xpos = cursorX + glyph->bearing.x * scale;
      const float ypos = cursorY + glyph->bearing.y * scale;
      const float w = glyph->size.x * scale;
      const float h = glyph->size.y * scale;

      const glm::vec2 tl{xpos, ypos};
      const glm::vec2 tr{xpos + w, ypos};
      const glm::vec2 bl{xpos, ypos + h};
      const glm::vec2 br{xpos + w, ypos + h};

      const glm::vec2 uvTL{glyph->uvMin.x, glyph->uvMax.y};
      const glm::vec2 uvTR{glyph->uvMax.x, glyph->uvMax.y};
      const glm::vec2 uvBL{glyph->uvMin.x, glyph->uvMin.y};
      const glm::vec2 uvBR{glyph->uvMax.x, glyph->uvMin.y};

      outVertices.push_back({tl, uvTL});
      outVertices.push_back({bl, uvBL});
      outVertices.push_back({br, uvBR});

      outVertices.push_back({tl, uvTL});
      outVertices.push_back({br, uvBR});
      outVertices.push_back({tr, uvTR});

      cursorX += glyph->advance * scale;
    }

    cursorY -= lineHeight;
  }
}

} // namespace engine::graphics
