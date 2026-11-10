#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string_view>

namespace engine::asset {

class AssetHandle {
public:
  using ValueType = std::uint64_t;

  AssetHandle() noexcept = default;

  /**
   * @brief Allocates a fresh, globally-unique handle
   */
  static AssetHandle Generate();

  /*
   * @brief Parses the canonical string form. Returns Null() if malformed
   */
  static AssetHandle FromString(std::string_view str);

  /**
   * @brief Returns the null handle. Same as default construction
   */
  static AssetHandle Null() noexcept { return AssetHandle{}; }

  bool IsNull() const noexcept { return m_value == 0; }
  [[nodiscard]] ValueType Value() const noexcept { return m_value; }
  std::string ToString() const;

  bool operator==(const AssetHandle &other) const noexcept {
    return m_value == other.m_value;
  }

  bool operator!=(const AssetHandle &other) const noexcept {
    return m_value != other.m_value;
  }

  bool operator<(const AssetHandle &other) const noexcept {
    return m_value < other.m_value;
  }

private:
  explicit AssetHandle(ValueType value) noexcept : m_value(value) {}

  ValueType m_value = 0;
};

} // namespace engine::asset

namespace std {
template <> struct hash<engine::asset::AssetHandle> {
  std::size_t
  operator()(const engine::asset::AssetHandle &handle) const noexcept {
    return std::hash<engine::asset::AssetHandle::ValueType>{}(handle.Value());
  }
};
} // namespace std
