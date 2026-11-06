#include "engine/asset/finalizer/SoundFinalizer.hpp"
#include "engine/asset/importer/sound/SoundImportResult.hpp"
#include "engine/audio/assets/Sound.hpp"
#include "engine/utils/Logger.hpp"
#include <memory>

namespace engine::asset {

std::shared_ptr<Asset> SoundFinalizer::Finalize(AssetHandle handle,
                                                const SoundImportResult &data) {
  auto sound = std::make_shared<audio::SoundAsset>(handle);
  sound->sourcePath = data.absolutePath;
  sound->sampleRate = data.sampleRate;
  sound->channels = data.channels;
  sound->frameCount = data.frameCount;

  if (!sound) {
    Logger::Error("[SoundFinalizer] Audio::CreateSound failed for %s",
                  data.absolutePath.c_str());
    return nullptr;
  }
  return std::static_pointer_cast<Asset>(std::move(sound));
}

} // namespace engine::asset
