#pragma once

#include "engine/ecs/Registry.hpp"

#include <cstddef>
#include <limits>
#include <tuple>
#include <typeindex>
#include <utility>
#include <vector>

namespace engine::ecs {

// View over one or more components.
// Iteration selects the SMALLEST sparse set as leader -> O(min(|A|,|B|,...)).
// Without<...>() returns a new View by value (safe for chained range-for).
template <Component... Ts> class View {
  static_assert(sizeof...(Ts) > 0);

public:
  explicit View(Registry &registry) : registry_(&registry) {
    storages_ = std::make_tuple(&registry.template StorageOf<Ts>()...);
    FindLeader();
  }

  template <Component... Xs> [[nodiscard]] View Without() const {
    View r = *this;
    (r.excluded_.push_back(std::type_index(typeid(Xs))), ...);
    return r;
  }

  [[nodiscard]] const std::size_t Size() const {
    return leader_snapshot_.size();
  }

  class Iterator {
  public:
    Iterator(View *v, std::size_t i) : view_(v), idx_(i) { SkipInvalid(); }

    [[nodiscard]] auto operator*() const {
      Entity e = view_->leader_snapshot_[idx_];
      return Deref(e, std::index_sequence_for<Ts...>{});
    }

    Iterator &operator++() {
      ++idx_;
      SkipInvalid();
      return *this;
    }
    bool operator==(const Iterator &o) const { return idx_ == o.idx_; }
    bool operator!=(const Iterator &o) const { return idx_ != o.idx_; }

  private:
    template <std::size_t... Is>
    [[nodiscard]] std::tuple<Entity, Ts &...>
    Deref(Entity e, std::index_sequence<Is...>) const {
      return {e, *std::get<Is>(view_->storages_)->TryGet(e)...};
    }

    void SkipInvalid() {
      while (idx_ < view_->leader_snapshot_.size() &&
             !view_->Matches(view_->leader_snapshot_[idx_])) {
        ++idx_;
      }
    }

    View *view_;
    std::size_t idx_;
  };

  Iterator begin() {
    FindLeader();
    return Iterator(this, 0);
  }

  Iterator end() { return Iterator(this, leader_snapshot_.size()); }

private:
  void FindLeader() {
    leader_snapshot_.clear();
    std::size_t best = std::numeric_limits<std::size_t>::max();

    IStorage *best_storage = nullptr;
    std::apply(
        [&](auto *...s) {
          auto check = [&](auto *st) {
            if (st->Size() < best) {
              best = st->Size();
              best_storage = st;
            }
          };
          (check(s), ...);
        },
        storages_);

    if (best_storage != nullptr && best > 0 &&
        best != std::numeric_limits<std::size_t>::max()) {
      FillLeaderSnapshot(best_storage);
    }
  }

  void FillLeaderSnapshot(IStorage *storage) {
    std::apply(
        [&](auto *...s) {
          ((s == storage ? (leader_snapshot_.assign(s->Entities(),
                                                    s->Entities() + s->Size()))
                         : (void)0),
           ...);
        },
        storages_);
  }

  [[nodiscard]] bool Matches(Entity e) const {
    bool all = std::apply([&](auto *...s) { return (s->Contains(e) && ...); },
                          storages_);
    if (!all)
      return false;
    for (auto ti : excluded_) {
      const auto *st = registry_->TryStorageByType(ti);
      if (st && st->Contains(e))
        return false;
    }
    return true;
  }

  Registry *registry_;
  std::tuple<SparseSet<Ts> *...> storages_;
  std::vector<std::type_index> excluded_;
  std::vector<Entity> leader_snapshot_;
};

} // namespace engine::ecs
