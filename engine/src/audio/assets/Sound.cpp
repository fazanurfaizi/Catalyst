#include "engine/audio/assets/Sound.hpp"
#include "engine/audio/AudioEngine.hpp"

namespace engine::audio {

SoundAsset::~SoundAsset() { AudioEngine::Get().Shutdown(); }

} // namespace engine::audio
