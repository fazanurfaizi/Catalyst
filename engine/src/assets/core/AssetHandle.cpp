#include "engine/asset/core/AssetHandle.hpp"
#include "engine/utils/Id.hpp"

#include <cstdio>

namespace engine::asset {

AssetHandle AssetHandle::Generate() {
  Id id = Id();
  return AssetHandle(static_cast<ValueType>(id));
}

AssetHandle AssetHandle::FromString(std::string_view str) {
  if (str.size() != 16) {
    return Null();
  }
  ValueType value = 0;
  if (std::sscanf(std::string(str).c_str(), "%llx",
                  reinterpret_cast<unsigned long long *>(&value)) != 1) {
    return Null();
  }
  return AssetHandle(value);
}

std::string AssetHandle::ToString() const {
  char buffer[17];
  std::snprintf(buffer, sizeof(buffer), "%016llx",
                static_cast<unsigned long long>(m_value));
  return std::string(buffer);
}

} // namespace engine::asset
