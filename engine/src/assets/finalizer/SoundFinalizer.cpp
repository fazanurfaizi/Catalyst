#include "engine/asset/finalizer/SoundFinalizer.hpp"
#include "engine/asset/importer/sound/SoundImportResult.hpp"
#include "engine/resources/SoundAsset.hpp"
#include "engine/utils/Logger.hpp"
#include <memory>

namespace engine::asset {

std::shared_ptr<Asset>
SoundFinalizer::Finalize(AssetHandle handle, const SoundImportResult &data,
                         [[maybe_unused]] bool eagerGpu) {
  auto sound = std::make_shared<resources::SoundAsset>(handle);
  sound->sourcePath = data.absolutePath;
  sound->sampleRate = data.sampleRate;
  sound->channels = data.channels;

  if (data.sampleRate > 0 && data.frameCount > 0) {
    sound->durationSeconds = static_cast<float>(data.frameCount) /
                             static_cast<float>(data.sampleRate);
  }

  if (!sound) {
    Logger::Error("[SoundFinalizer] Audio::CreateSound failed for %s",
                  data.absolutePath.c_str());
    return nullptr;
  }
  return std::static_pointer_cast<Asset>(std::move(sound));
}

} // namespace engine::asset
