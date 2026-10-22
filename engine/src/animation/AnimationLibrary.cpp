#include "engine/animation/AnimationLibrary.hpp"

#include "engine/animation/AnimationClip.hpp"
#include "engine/core/Logger.hpp"
#include <string>
#include <utility>

namespace engine {

void AnimationLibrary::Add(AnimationClip clip) {
  if (clip.name.empty()) {
    Logger::Warn("[AnimationLibrary] Add() called with empty name - ignored");
    return;
  }

  if (!clip.IsValid()) {
    Logger::Warn("[AnimationLibrary] clip '%s' has no frames - ignored",
                 clip.name.c_str());
    return;
  }

  m_clips[clip.name] = std::move(clip);
}

bool AnimationLibrary::AddStrip(const std::string &clipName,
                                const TextureAtlas &atlas,
                                const std::string &regionPrefix, int frameCount,
                                float frameDurationSec, bool loop) {
  if (frameCount <= 0 || frameDurationSec <= 0.0f) {
    Logger::Error("[AnimationLibrary] AddStrip invalid args: %s",
                  clipName.c_str());
    return false;
  }

  AnimationClip clip;
  clip.name = clipName;
  clip.loop = loop;
  clip.frames.reserve(static_cast<std::size_t>(frameCount));

  for (int i = 0; i < frameCount; ++i) {
    const std::string regionName = regionPrefix + "_" + std::to_string(i);
    const AtlasRegion *r = atlas.GetRegion(regionName);
    if (!r) {
      Logger::Error(
          "[AnimationLibrary] mission atlas region '%s' for clip '%s'",
          regionName.c_str(), clipName.c_str());
      return false;
    }
    AnimationFrame frame;
    frame.uvMin = r->uvMin;
    frame.uvMax = r->uvMax;
    frame.durationSec = frameDurationSec;
    clip.frames.push_back(frame);
  }

  m_clips[clipName] = std::move(clip);
  return true;
}

const AnimationClip *AnimationLibrary::Get(const std::string &name) const {
  auto it = m_clips.find(name);
  return it == m_clips.end() ? nullptr : &it->second;
}

} // namespace engine
