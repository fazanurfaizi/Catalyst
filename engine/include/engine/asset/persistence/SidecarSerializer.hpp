#pragma once

#include "engine/asset/importer/ImportSettings.hpp"

#include <cstdint>
#include <filesystem>
#include <memory>

namespace engine::asset {

class IAssetImporter;

std::unique_ptr<ImportSettings>
LoadSidecarSettings(const std::filesystem::path &assetPath,
                    const IAssetImporter *importer,
                    std::uint64_t *outHash = nullptr);

} // namespace engine::asset
