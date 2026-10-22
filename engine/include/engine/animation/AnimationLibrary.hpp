#pragma once

#include "engine/animation/AnimationClip.hpp"
#include "engine/graphics/TextureAtlas.hpp"

#include <string>
#include <unordered_map>

namespace engine {

class AnimationLibrary {
public:
  AnimationLibrary() = default;
  AnimationLibrary(const AnimationLibrary &) = delete;
  AnimationLibrary &operator=(const AnimationLibrary &) = delete;
  AnimationLibrary(AnimationLibrary &&) noexcept = default;
  AnimationLibrary &operator=(AnimationLibrary &&) noexcept = default;

  void Add(AnimationClip clip);

  bool AddStrip(const std::string &clipName, const TextureAtlas &atlas,
                const std::string &regionPrefix, int frameCount,
                float frameDurationSec, bool loop = true);

  const AnimationClip *Get(const std::string &name) const;
  int Count() const { return static_cast<int>(m_clips.size()); }

private:
  std::unordered_map<std::string, AnimationClip> m_clips;
};

} // namespace engine
