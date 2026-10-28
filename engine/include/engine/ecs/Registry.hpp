#pragma once

#include "SparseSet.hpp"
#include "engine/utils/Logger.hpp"

#include <cassert>
#include <cstdint>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace engine::ecs {

template <Component... Ts> class View; // fwd

class Registry {
public:
  Registry() = default;
  Registry(const Registry &) = delete;
  Registry &operator=(const Registry &) = delete;
  Registry(Registry &&) = default;
  Registry &operator=(Registry &&) = default;

  // Entity lifecycle
  [[nodiscard]] Entity Create() {
    std::uint32_t index;
    if (!free_.empty()) {
      index = free_.back();
      free_.pop_back();
    } else {
      index = next_index_++;
      generations_.push_back(0);
      alive_.push_back(0);
    }
    alive_[index] = 1;
    return Entity{index, generations_[index]};
  }

  void Destroy(Entity e) noexcept {
    if (!IsAlive(e))
      return;
    for (auto &[ti, storage] : storages_) {
      storage->Remove(e);
    }

    alive_[e.index] = 0;
    ++generations_[e.index];
    free_.push_back(e.index);
  }

  void DestroyDeferred(Entity e) {
    if (IsAlive(e))
      pending_destroy_.push_back(e);
  }

  void FlushDeferred() {
    for (auto e : pending_destroy_) {
      Destroy(e);
    }
    pending_destroy_.clear();
  }

  [[nodiscard]] bool IsAlive(Entity e) const noexcept {
    return e.index < generations_.size() && alive_[e.index] &&
           generations_[e.index] == e.generation;
  }

  [[nodiscard]] std::size_t EntityCount() const noexcept {
    return next_index_ - free_.size();
  }

  // Component
  template <Component T, typename... Args>
  T &Emplace(Entity e, Args &&...args) {
    assert(IsAlive(e) && "Emplace on dead entity");
    return StorageOf<T>().Emplace(e, std::forward<Args>(args)...);
  }

  template <Component T, typename... Args>
  T &GetOrEmplace(Entity e, Args &&...args) {
    if (T *existing = TryGet<T>(e))
      return *existing;
    return Emplace<T>(e, std::forward<Args>(args)...);
  }

  template <Component T> void Remove(Entity e) noexcept {
    if (auto *s = TryStorageOf<T>())
      s->Remove(e);
  }

  template <Component T> [[nodiscard]] T *TryGet(Entity e) noexcept {
    auto *s = TryStorageOf<T>();
    return s ? s->TryGet(e) : nullptr;
  }

  template <Component T>
  [[nodiscard]] const T *TryGet(Entity e) const noexcept {
    auto *s = TryStorageOf<T>();
    return s ? s->TryGet(e) : nullptr;
  }

  template <Component T> [[nodiscard]] T &Get(Entity e) {
    T *p = TryGet<T>(e);
    assert(p && "Get<T>: missing component");
    return *p;
  }

  template <Component T> [[nodiscard]] bool Has(Entity e) const noexcept {
    auto *s = TryStorageOf<T>();
    return s && s->Contains(e);
  }

  // View
  template <Component... Ts> [[nodiscard]] View<Ts...> View();

  // Internal
  template <Component T> SparseSet<T> &StorageOf() {
    auto key = std::type_index(typeid(T));
    auto it = storages_.find(key);
    if (it == storages_.end()) {
      auto ptr = std::make_unique<SparseSet<T>>();
      auto *raw = ptr.get();
      storages_.emplace(key, std::move(ptr));
      return *raw;
    }
    return *static_cast<SparseSet<T> *>(it->second.get());
  }

  template <Component T> [[nodiscard]] SparseSet<T> *TryStorageOf() noexcept {
    auto it = storages_.find(std::type_index(typeid(T)));
    if (it == storages_.end())
      return nullptr;
    return static_cast<SparseSet<T> *>(it->second.get());
  }

  template <Component T>
  [[nodiscard]] const SparseSet<T> *TryStorageOf() const noexcept {
    auto it = storages_.find(std::type_index(typeid(T)));
    if (it == storages_.end())
      return nullptr;
    return static_cast<SparseSet<T> *>(it->second.get());
  }

  [[nodiscard]] IStorage *TryStorageByType(std::type_index ti) noexcept {
    auto it = storages_.find(ti);
    return it == storages_.end() ? nullptr : it->second.get();
  }

  [[nodiscard]] const IStorage *
  TryStorageByType(std::type_index ti) const noexcept {
    auto it = storages_.find(ti);
    return it == storages_.end() ? nullptr : it->second.get();
  }

private:
  std::vector<std::uint32_t> generations_;
  std::vector<std::uint8_t> alive_;
  std::vector<std::uint32_t> free_;
  std::uint32_t next_index_ = 0;

  std::unordered_map<std::type_index, std::unique_ptr<IStorage>> storages_;
  std::vector<Entity> pending_destroy_;
};
} // namespace engine::ecs

#include "View.hpp"

namespace engine::ecs {
template <Component... Ts> [[nodiscard]] inline View<Ts...> Registry::View() {
  return engine::ecs::View<Ts...>(*this);
}
} // namespace engine::ecs
