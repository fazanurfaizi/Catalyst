#include "engine/asset/importer/sound/SoundImporter.hpp"
#include "engine/asset/importer/sound/SoundImportResult.hpp"
#include "extras/nodes/ma_channel_separator_node/ma_channel_separator_node.h"

namespace engine::asset {

namespace {
ImportResult<ImportedAssetDataPtr> ImportSound(const ImportRequest &request) {
  std::error_code ec;
  if (!std::filesystem::exists(request.absolutePath, ec) || ec) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::FileNotFound,
                          "Sound file not found")
            .WithPath(request.absolutePath));
  }

  if (std::filesystem::is_directory(request.absolutePath)) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::MalformedContent,
                          "Path is a directory, not a sound file")
            .WithPath(request.absolutePath));
  }

  ma_decoder decoder;
  ma_decoder_config config = ma_decoder_config_init_default();
  if (ma_decoder_init_file(request.absolutePath.string().c_str(), &config,
                           &decoder) != MA_SUCCESS) {
    return ImportResult<ImportedAssetDataPtr>::Err(
        ImportError::Make(ImportError::Code::DecodeFailed,
                          "miniaudio could not open the file")
            .WithPath(request.absolutePath));
  }

  ma_uint64 frameCount = 0;
  ma_decoder_get_length_in_pcm_frames(&decoder, &frameCount);

  auto result = std::make_unique<SoundImportResult>();
  result->absolutePath = request.absolutePath;
  result->sampleRate = decoder.outputSampleRate;
  result->channels = decoder.outputChannels;
  result->frameCount = static_cast<std::uint32_t>(frameCount);

  ma_decoder_uninit(&decoder);
  return ImportResult<ImportedAssetDataPtr>::Ok(std::move(result));
}
} // namespace

ImportResult<ImportedAssetDataPtr>
SoundImporter::Import(const ImportRequest &request) const {
  return ImportSound(request);
}

} // namespace engine::asset
