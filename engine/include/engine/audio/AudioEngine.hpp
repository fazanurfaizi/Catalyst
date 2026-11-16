#pragma once

#include "engine/resources/SoundAsset.hpp"
#include <string>

struct ma_engine;
struct ma_sound;

namespace engine::audio {

class AudioEngine {
public:
  static AudioEngine &Get();

  AudioEngine(const AudioEngine &) = delete;
  AudioEngine &operator=(const AudioEngine &) = delete;

  bool Init();
  void Update();
  void Shutdown();
  bool IsInitialized() const;

  void PlayOneShot(const resources::SoundAsset &sound, float volume = 1.0f);

  /** Path-based convenience, resolved against ASSETS_PATH. */
  void PlaySoundPath(const std::string &path, float volume = 1.0f);

  void SetMasterVolume(float volume);

  ma_engine *GetHandle() const;

  static void OnSoundFinished(void *data, ma_sound *sound);

  struct Impl;
  Impl *m_impl = nullptr;

protected:
  AudioEngine();
  ~AudioEngine();
};

} // namespace engine::audio
