#pragma once

#include "engine/components/AnimationClip.hpp"
#include "engine/graphics/TextureAtlas.hpp"

#include <string>
#include <unordered_map>

namespace engine::assets {

class AnimationLibrary {
public:
  AnimationLibrary() = default;
  AnimationLibrary(const AnimationLibrary &) = delete;
  AnimationLibrary &operator=(const AnimationLibrary &) = delete;
  AnimationLibrary(AnimationLibrary &&) noexcept = default;
  AnimationLibrary &operator=(AnimationLibrary &&) noexcept = default;

  void Add(components::AnimationClip clip);

  bool AddStrip(const std::string &clipName,
                const graphics::TextureAtlas &atlas,
                const std::string &regionPrefix, int frameCount,
                float frameDurationSec, bool loop = true);

  const components::AnimationClip *Get(const std::string &name) const;
  int Count() const { return static_cast<int>(m_clips.size()); }

private:
  std::unordered_map<std::string, components::AnimationClip> m_clips;
};

} // namespace engine::assets
