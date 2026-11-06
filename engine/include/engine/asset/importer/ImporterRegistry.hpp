#pragma once

#include "engine/asset/core/AssetType.hpp"
#include "engine/asset/importer/IAssetImporter.hpp"
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
namespace engine::asset {

class ImporterRegistry {
public:
  ImporterRegistry() = default;
  ~ImporterRegistry() = default;

  ImporterRegistry(ImporterRegistry &&) = default;
  ImporterRegistry(const ImporterRegistry &) = delete;
  ImporterRegistry &operator=(ImporterRegistry &&) = default;
  ImporterRegistry &operator=(const ImporterRegistry &) = delete;

  void Register(std::unique_ptr<IAssetImporter> importer);

  IAssetImporter *FindForExtension(std::string_view extension) const;
  IAssetImporter *FindForAssetType(AssetType type) const;

  const std::vector<std::unique_ptr<IAssetImporter>> &GetAll() const {
    return m_importers;
  }

  std::size_t GetImporterCount() const { return m_importers.size(); }

  static std::string NormalizeExtension(std::string_view extension);

private:
  std::vector<std::unique_ptr<IAssetImporter>> m_importers;
  std::unordered_map<std::string, IAssetImporter *> m_byExtension;
  std::unordered_map<AssetType, IAssetImporter *> m_byType;
};

} // namespace engine::asset
