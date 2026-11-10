#pragma once

#include "engine/asset/core/ImportError.hpp"
#include <cassert>
#include <type_traits>
#include <utility>
#include <variant>

namespace engine::asset {

namespace detail {
template <typename T> struct IsNullable : std::false_type {};
template <typename T> struct IsNullable<T *> : std::true_type {};
template <typename T> struct IsNullable<std::unique_ptr<T>> : std::true_type {};
template <typename T> struct IsNullable<std::shared_ptr<T>> : std::true_type {};
} // namespace detail

template <typename T> class ImportResult {
public:
  static ImportResult Ok(T value) {
    if constexpr (detail::IsNullable<T>::value) {
      if (value == nullptr) {
        assert(false &&
               "[ImportResult] Ok called with nullptr payload - an importer "
               "that succeeds must produce data; return Err(...) instead");
        return Err(ImportError::Make(
            ImportError::Code::Internal,
            "Ok(nullptr) - importer succeeded without producing data"));
      }
    }
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

  T &Value() {
    assert(IsOk() && "ImportResult::Value() on non-Ok result");
    return std::get<1>(m_data);
  }
  const T &Value() const {
    assert(IsOk() && "ImportResult::Value() on non-Ok result");
    return std::get<1>(m_data);
  }
  const ImportError &Error() const {
    assert(IsError() && "ImportResult::Error() on non-Error result");
    return std::get<2>(m_data);
  }
  T ValueOf(T fallback) const {
    return IsOk() ? std::get<1>(m_data) : std::move(fallback);
  }

  /**
   * Ergonomic consumption, exactly one of the callbacks runs.
   * Return type must be compatible (or both void).
   */
  template <typename OnOk, typename OnErr>
  auto Match(OnOk &&onOk, OnErr &&onErr) {
    if (IsOk())
      return std::forward<OnOk>(onOk)(std::get<1>(m_data));
    return std::forward<OnErr>(onErr)(std::get<2>(m_data));
  }

private:
  ImportResult() = default;
  std::variant<std::monostate, T, ImportError> m_data;
};

} // namespace engine::asset
