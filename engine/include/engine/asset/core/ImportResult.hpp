#pragma once

#include "engine/asset/core/ImportError.hpp"
#include <utility>
#include <variant>

namespace engine::asset {

template <typename T> class ImportResult {
public:
  static ImportResult Ok(T value) {
    ImportResult result;
    result.m_data.template emplace<1>(std::move(value));
    return result;
  }

  static ImportResult Err(ImportError error) {
    ImportResult result;
    result.m_data.template emplace<2>(std::move(error));
    return result;
  }

  bool IsOk() const noexcept { return m_data.index() == 1; }
  bool IsError() const noexcept { return m_data.index() == 2; }

  T &Value() { return std::get<1>(m_data); }
  const T &Value() const { return std::get<1>(m_data); }
  const ImportError &Error() const { return std::get<2>(m_data); }

private:
  ImportResult() = default;
  std::variant<std::monostate, T, ImportError> m_data;
};

} // namespace engine::asset
