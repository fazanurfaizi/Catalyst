#pragma once

#include "engine/audio/assets/Sound.hpp"
#include <string>

struct ma_engine;

namespace engine::audio {

class AudioEngine {
public:
  static AudioEngine &Get();

  AudioEngine(const AudioEngine &) = delete;
  AudioEngine &operator=(const AudioEngine &) = delete;

  bool Init();
  void Shutdown();
  bool IsInitialized() const;

  void PlayOneShot(const SoundAsset &sound, float volume = 1.0f);

  /** Path-based convenience, resolved against ASSETS_PATH. */
  void PlaySoundPath(const std::string &path, float volume = 1.0f);

  void SetMasterVolume(float volume);

  ma_engine *GetHandle() const;

private:
  AudioEngine();
  ~AudioEngine();

  struct Impl;
  Impl *m_impl = nullptr;
};

} // namespace engine::audio
