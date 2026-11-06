#pragma once

#include "engine/asset/core/AssetType.hpp"
#include <cstdint>
#include <string_view>
namespace engine::asset {

class ImportSettings {
public:
  virtual ~ImportSettings() = default;

  virtual AssetType GetAssetType() const = 0;
  virtual std::string_view GetImporterId() const = 0;
  virtual std::uint32_t GetSchemaVersion() const = 0;
  virtual std::uint64_t ComputeHash() const = 0;
};

} // namespace engine::asset
