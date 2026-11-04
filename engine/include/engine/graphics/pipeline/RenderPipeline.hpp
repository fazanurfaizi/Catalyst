#pragma once

#include "engine/graphics/pipeline/RenderStage.hpp"
#include "engine/utils/Logger.hpp"

#include <cstdint>

namespace engine::graphics {

class RenderPipeline {
public:
  void Reset() noexcept {
    m_available = AttachNone;
    m_current = RenderStage::Sprite;
    m_inStage = false;
  }

  [[nodiscard]] bool BeginStage(RenderStage s) noexcept {
    if (m_inStage) {
      Logger::Error("[RenderPipeline] BeginStage(%s) while %s active",
                    StageName(s), StageName(m_current));
      return false;
    }

    if (s == RenderStage::Blit) {
      constexpr std::uint32_t kSources = AttachSceneColor | AttachLightColor;
      if ((m_available & kSources) == 0u) {
        Logger::Error("[RenderPipeline] Blit needs SceneColor or LightColor");
        return false;
      }
    } else {
      const auto io = StageInfoOf(s);
      if ((m_available & io.requiresMask) != io.requiresMask) {
        Logger::Error("[RenderPipeline] %s requires 0x%x, available 0x%x",
                      StageName(s), io.requiresMask, m_available);
        return false;
      }
    }

    m_current = s;
    m_inStage = true;
    return true;
  }

  void EndStage(RenderStage s) noexcept {
    if (!m_inStage || m_current != s) {
      Logger::Error("[RenderPipeline] EndStage(%s) mismatch (current=%s)",
                    StageName(s), StageName(m_current));
      return;
    }
    m_available |= StageInfoOf(s).produces;
    m_inStage = false;
  }

  [[nodiscard]] bool HasAttachment(Attachment a) const noexcept {
    return (m_available & a) == a;
  }

  [[nodiscard]] std::uint32_t AvailableAttachments() const noexcept {
    return m_available;
  }

  [[nodiscard]] RenderStage current() const noexcept { return m_current; }
  [[nodiscard]] bool InStage() const noexcept { return m_inStage; }

private:
  std::uint32_t m_available = AttachNone;
  RenderStage m_current = RenderStage::Sprite;
  bool m_inStage = false;
};

} // namespace engine::graphics
