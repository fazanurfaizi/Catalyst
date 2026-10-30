#pragma once

#include "engine/graphics/Font.hpp"
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace engine::graphics {

enum class TextAlignment {
  Left,
  Center,
  Right,
};

struct Vertex {
  glm::vec2 position;
  glm::vec2 texCoords;
};

struct LineInfo {
  std::string text;
  float width;
};

struct LayoutMetrics {
  std::vector<LineInfo> lines;
  float maxLineWidth = 0.0f;
  float lineHeight = 0.0f;
  float totalHeight = 0.0f;
};

class TextLayout {
public:
  static float MeasureStringWidth(const Font &font, const std::string &text,
                                  float scale);

  static std::vector<LineInfo> WrapText(const Font &font,
                                        const std::string &text, float maxWidth,
                                        float scale);

  static LayoutMetrics ComputeLayout(const Font &font, const std::string &text,
                                     float maxWidth, float scale);

  static void BuildTextBatch(const Font &font, const std::string &text,
                             float maxWidth, float scale,
                             TextAlignment alignment,
                             std::vector<Vertex> &outVertices,
                             const glm::vec2 &origin = glm::vec2(0.0f));
};

} // namespace engine::graphics
