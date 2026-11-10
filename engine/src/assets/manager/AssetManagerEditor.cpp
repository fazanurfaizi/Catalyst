#include "engine/asset/manager/AssetManagerEditor.hpp"
#include "engine/utils/Logger.hpp"
#include <utility>

namespace engine::asset {

AssetHandle
AssetManagerEditor::ImportAsset(const std::filesystem::path &relativePath,
                                std::string name) {
  for (const auto &[handle, record] : m_records) {
    if (record.path == relativePath) {
      return handle;
    }
  }

  auto *importer =
      m_registry.FindForExtension(relativePath.extension().string());
  if (!importer) {
    Logger::Error(
        "[AssetManagerEditor] ImportAsset no importer for extension '%s'",
        relativePath.extension().string().c_str());
    return AssetHandle::Null();
  }

  const auto handle = AssetHandle::Generate();
  AddRecord(AssetRecord(handle, importer->GetAssetType(), relativePath,
                        std::move(name)));
  return handle;
}

} // namespace engine::asset
