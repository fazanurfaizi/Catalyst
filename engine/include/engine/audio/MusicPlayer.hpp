#pragma once

#include <string>

struct ma_engine;
struct ma_sound;

namespace engine::audio {

class MusicPlayer {
public:
  MusicPlayer();
  ~MusicPlayer();

  MusicPlayer(const MusicPlayer &) = default;
  MusicPlayer &operator=(const MusicPlayer &) = default;

  bool Load(const std::string &path);
  void Play();
  void Stop();
  void SetVolume(float volume);
  void SetLooping(bool loop);

private:
  ma_sound *m_sound = nullptr;
  bool m_loaded = false;
};

} // namespace engine::audio
