#pragma once

#include "Entity.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace engine::ecs {

template <typename T>
concept Component =
    std::is_object_v<T> && !std::is_const_v<T> && !std::is_volatile_v<T>;

// Minimal erased interface type for Registry. Virtual calls only occur
// on Destroy/Emplace/Remove per entity
class IStorage {
public:
  virtual ~IStorage() = default;
  virtual bool Contains(Entity e) const noexcept = 0;
  virtual std::size_t Size() const noexcept = 0;
  virtual void Remove(Entity e) noexcept = 0;
  virtual void Clear() noexcept = 0;
};

namespace detail {
inline constexpr std::uint32_t kEmpty =
    std::numeric_limits<std::uint32_t>::max();
}

// Sparse set: component data is stored contiguously in `dense_`.
// sparse_[entity.index] -> index in dense_ (or kEmpty)
// entities_[i] -> entity in the i-th dense_ slot
// dense_[i] -> T for entities_[i]
//
// Iteration O(Size()), lookup O(1), insert/remove O(1) amortized.
template <Component T> class SpareSet final : public IStorage {
public:
  using value_type = T;

  // IStorage
  [[nodiscard]] bool Contains(Entity e) const noexcept override {
    return e.index < spare_.size() && spare_[e.index] != detail::kEmpty;
  }

  [[nodiscard]] std::size_t Size() const noexcept override {
    return dense_.size();
  }

  void Clear() noexcept override {
    spare_.clear();
    entities_.clear();
    dense_.clear();
  }

  void Remove(Entity e) noexcept override {
    if (!Contains(e))
      return;
    const auto di = spare_[e.index];
    const auto last = static_cast<std::uint32_t>(dense_.size() - 1);

    if (di != last) {
      dense_[di] = std::move(dense_[last]);
      entities_[di] = entities_[last];
      spare_[entities_[di].index] = di;
    }

    dense_.pop_back();
    entities_.pop_back();
    spare_[e.index] = detail::kEmpty;
  }

  [[nodiscard]] T *TryGet(Entity e) noexcept {
    return Contains(e) ? &dense_[spare_[e.index]] : nullptr;
  }

  [[nodiscard]] const T *TryGet(Entity e) const noexcept {
    return Contains(e) ? &dense_[spare_[e.index]] : nullptr;
  }

  template <typename... Args> T &Emplace(Entity e, Args &&...args) {
    if (Contains(e)) {
      T *slot = &dense_[spare_[e.index]];
      std::destroy_at(slot);
      std::construct_at(slot, std::forward<Args>(args)...);
      return *slot;
    }
    if (e.index >= spare_.size()) {
      spare_.resize(static_cast<std::size_t>(e.index) + 1, detail::kEmpty);
    }

    spare_[e.index] = static_cast<std::size_t>(dense_.size());
    entities_.push_back(e);
    dense_.emplace_back(std::forward<Args>(args)...);
    return dense_.back();
  }

  [[nodiscard]] Entity *Entities() noexcept { return entities_.data(); }
  [[nodiscard]] const Entity *Entities() const noexcept {
    return entities_.data();
  }
  [[nodiscard]] T *Data() noexcept { return dense_.data(); }
  [[nodiscard]] const T *Data() const noexcept { return dense_.data(); }

private:
  std::vector<std::uint32_t> spare_;
  std::vector<Entity> entities_;
  std::vector<T> dense_;
};

} // namespace engine::ecs
