#pragma once

#include <string>

namespace engine::audio {

class AudioEngine {
public:
  AudioEngine();
  ~AudioEngine();

  AudioEngine(const AudioEngine &) = delete;
  AudioEngine &operator=(const AudioEngine &) = delete;

  bool Init();
  void Shutdown();

  // Play a sound effect once (fire-and-forget)
  void PlaySound(const std::string &filepath);

  // Load a music track and play it in a loop (BGM)
  bool PlayMusic(const std::string &filepath);
  void StopMusic();
  void SetMasterVolume(float volume);

private:
  struct Impl;
  Impl *m_impl = nullptr;
};

} // namespace engine::audio
