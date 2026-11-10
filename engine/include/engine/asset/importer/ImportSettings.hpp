#pragma once

#include "engine/asset/core/AssetType.hpp"

#include <cstdint>
#include <cstring>
#include <string_view>

#include <nlohmann/json.hpp>

namespace engine::asset {

using json = nlohmann::json;

class ImportSettings {
public:
  virtual ~ImportSettings() = default;

  virtual AssetType GetAssetType() const = 0;
  virtual std::string_view GetImporterId() const = 0;
  virtual std::uint32_t GetSchemaVersion() const = 0;
  virtual std::uint64_t ComputeHash() const = 0;

  virtual json ToJson() const = 0;
  virtual void FromJson(const json &j) = 0;
};

namespace detail {

inline constexpr std::uint64_t kHashSeed = 1469598103934665603ULL;

inline void HashCombine(std::uint64_t &h, std::uint64_t v) noexcept {
  h ^= v + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
}

inline void HashCombine(std::uint64_t &h, std::uint32_t v) noexcept {
  HashCombine(h, static_cast<std::uint64_t>(v));
}

inline void HashCombine(std::uint64_t &h, int v) noexcept {
  HashCombine(h, static_cast<std::uint64_t>(static_cast<std::uint32_t>(v)));
}

inline void HashCombine(std::uint64_t &h, bool v) noexcept {
  HashCombine(h, static_cast<std::uint64_t>(v ? 1u : 0u));
}

inline void HashCombine(std::uint64_t &h, float v) noexcept {
  std::uint32_t bits;
  std::memcpy(&bits, &v, sizeof(bits));
  HashCombine(h, bits);
}

} // namespace detail

} // namespace engine::asset
