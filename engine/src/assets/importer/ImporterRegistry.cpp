#include "engine/asset/importer/ImporterRegistry.hpp"
#include "engine/utils/Logger.hpp"

#include <cassert>

namespace engine::asset {

std::string ImporterRegistry::NormalizeExtension(std::string_view extension) {
  // Strip leading dots.
  while (!extension.empty() && extension.front() == '.') {
    extension.remove_prefix(1);
  }
  // Lowercase.
  std::string result;
  result.reserve(extension.size() + 1);
  result.push_back('.');
  for (char c : extension) {
    result.push_back(
        static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
  }
  return result;
}

void ImporterRegistry::Register(std::unique_ptr<IAssetImporter> importer) {
  if (!importer) {
    Logger::Error("[ImporterRegistry] Register null pointer");
    return;
  }

  IAssetImporter *raw = importer.get();

  const AssetType type = raw->GetAssetType();
  if (type == AssetType::None) {
    Logger::Error(
        "[ImporterRegistry]: Register importer '%s' claims AssetType::None",
        raw->GetId());
    return;
  }
  if (m_byType.find(type) != m_byType.end()) {
    Logger::Error(
        "[ImporterRegistry]: Register type '%s' is already registered",
        AssetTypeToString(type));
    return;
  }

  std::vector<std::string> normalized;
  normalized.reserve(8);
  for (std::string_view ext : raw->GetSupportedExtensions()) {
    std::string n = NormalizeExtension(ext);
    if (n.size() <= 1) {
      Logger::Error(
          "[ImporterRegistry]: Register importer '%s' claims empty extension",
          raw->GetId());
      return;
    }
    if (m_byExtension.find(n) != m_byExtension.end()) {
      Logger::Error("[ImporterRegistry]: Register extension '%s' is already "
                    "claimed by another importer",
                    n.c_str());
      return;
    }
    normalized.push_back(std::move(n));
  }
  if (normalized.empty()) {
    Logger::Error(
        "[ImporterRegistry]: Register importer '%s' claims zero extensions",
        raw->GetId());
    return;
  }

  m_importers.push_back(std::move(importer));
  m_byType[type] = raw;
  for (auto &n : normalized) {
    m_byExtension[std::move(n)] = raw;
  }
}

IAssetImporter *
ImporterRegistry::FindForExtension(std::string_view extension) const {
  const std::string normalized = NormalizeExtension(extension);
  auto it = m_byExtension.find(normalized);
  return it == m_byExtension.end() ? nullptr : it->second;
}

IAssetImporter *ImporterRegistry::FindForAssetType(AssetType type) const {
  auto it = m_byType.find(type);
  return it == m_byType.end() ? nullptr : it->second;
}

} // namespace engine::asset
