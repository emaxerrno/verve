#pragma once
#include <list>
#include <mutex>

namespace verve {

// Memory range-lock manager.
//
// Note that this implementation has linear complexity in the number of locks
// held because it is just implemented as a linked list of locks. It should be
// reimplemented using an efficient container for intervals.
class LockManager {
 public:
  LockManager(uint64_t low, uint64_t high) :
    low_(low), high_(high)
  {}

  bool TryReadLock(uint64_t low, uint64_t high) {
    const auto new_lock = make_lock(low, high, true);
    std::lock_guard<std::mutex> lk(lock_);
    for (const auto& held : locks_) {
      if (held.overlaps(new_lock) && !held.read) {
        return false;
      }
    }
    locks_.emplace_back(std::move(new_lock));
    return true;
  }

  bool TryWriteLock(uint64_t low, uint64_t high) {
    const auto new_lock = make_lock(low, high, false);
    std::lock_guard<std::mutex> lk(lock_);
    for (const auto& held : locks_) {
      if (held.overlaps(new_lock)) {
        return false;
      }
    }
    locks_.emplace_back(std::move(new_lock));
    return true;
  }

 private:
  // [low, high]
  struct Lock {
    uint64_t low;
    uint64_t high;
    bool read;

    bool overlaps(const Lock& other) const {
      if (other.high < low || other.low > high) {
        return false;
      }
      return true;
    }
  };

  Lock make_lock(uint64_t low, uint64_t high, bool read) const {
    if (low >= high || low < low_ || high > high_) {
      throw std::out_of_range("invalid lock range");
    }
    return {low, high, read};
  }

  mutable std::mutex lock_;
  const uint64_t low_;
  const uint64_t high_;
  std::list<Lock> locks_;
};

}
