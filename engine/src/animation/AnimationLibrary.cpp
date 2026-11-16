#include "engine/animation/AnimationLibrary.hpp"

#include "engine/components/AnimationClip.hpp"
#include "engine/resources/TextureAtlasAsset.hpp"
#include "engine/utils/Logger.hpp"
#include <cassert>
#include <string>
#include <utility>

namespace engine::animation {

void AnimationLibrary::Add(components::AnimationClip clip) {
  if (clip.name.empty()) {
    engine::Logger::Warn(
        "[AnimationLibrary] Add() called with empty name - ignored");
    return;
  }

  if (!clip.IsValid()) {
    engine::Logger::Warn("[AnimationLibrary] clip '%s' has no frames - ignored",
                         clip.name.c_str());
    return;
  }

  m_clips[clip.name] = std::move(clip);
}

bool AnimationLibrary::AddStrip(const std::string &clipName,
                                const resources::TextureAtlasAsset &atlas,
                                const std::string &regionPrefix, int frameCount,
                                float frameDurationSec, bool loop) {
  if (frameCount <= 0 || frameDurationSec <= 0.0f) {
    engine::Logger::Error("[AnimationLibrary] AddStrip invalid args: %s",
                          clipName.c_str());
    return false;
  }

  components::AnimationClip clip;
  clip.name = clipName;
  clip.loop = loop;
  clip.frames.reserve(static_cast<std::size_t>(frameCount));

  for (int i = 0; i < frameCount; ++i) {
    const std::string regionName = regionPrefix + "_" + std::to_string(i);

    if (!atlas.GetRegion(regionName)) {
      engine::Logger::Error(
          "[AnimationLibrary] missing region '%s' for clip '%s'",
          regionName.c_str(), clipName.c_str());
      return false;
    }

    components::AnimationFrame frame;
    frame.regionName = regionName;
    frame.durationSec = frameDurationSec;
    clip.frames.push_back(frame);
  }

  m_clips[clipName] = std::move(clip);
  return true;
}

const components::AnimationClip *
AnimationLibrary::Get(std::string_view name) const {
  auto it = m_clips.find(name);
  return it != m_clips.end() ? &it->second : nullptr;
}

} // namespace engine::animation
