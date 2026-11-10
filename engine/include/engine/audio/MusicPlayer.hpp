#pragma once

#include <string>

struct ma_engine;
struct ma_sound;

namespace engine::audio {

class MusicPlayer {
public:
  MusicPlayer();
  ~MusicPlayer();

  MusicPlayer(const MusicPlayer &) = delete;
  MusicPlayer &operator=(const MusicPlayer &) = delete;

  MusicPlayer(MusicPlayer &&) noexcept;
  MusicPlayer &operator=(MusicPlayer &&) noexcept;

  bool Load(const std::string &path);
  void Play();
  void Stop();
  void SetVolume(float volume);
  void SetLooping(bool loop);

  bool IsPlaying() const;

private:
  ma_sound *m_sound = nullptr;
  bool m_loaded = false;
  bool m_loop = false;
  float m_volume = 1.0f;
};

} // namespace engine::audio
