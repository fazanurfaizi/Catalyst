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

bool SaveSidecarSettings(const std::filesystem::path &assetPath,
                         const IAssetImporter *importer,
                         const ImportSettings &settings);

/** Writes default settings. Overwrites an existing sidecar. */
bool SaveSidecarSettings(const std::filesystem::path &assetPath,
                         const IAssetImporter *importer);

/** Writes default settings only when the sidecar does not exist yet. */
bool EnsureSidecarSettings(const std::filesystem::path &assetPath,
                           const IAssetImporter *importer);

} // namespace engine::asset
