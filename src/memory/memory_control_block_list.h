#ifndef MEMORY_CONTROL_BLOCK_LIST_H
#define MEMORY_CONTROL_BLOCK_LIST_H

#include "memory_control_block.h"
#include "math/math.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <type_traits>

namespace hse::memory {

template<typename T>
concept MCBPredicate = std::is_nothrow_invocable_r_v<bool, T, const MemoryControlBlock*>;

template<MCBPredicate P>
MCBPredicate auto operator!(P p) {
  return [p](MemoryControlBlock *mcb) noexcept {
    return !p(mcb);
  };
}

template<MCBPredicate P1, MCBPredicate P2>
MCBPredicate auto operator&&(P1 p1, P2 p2) {
  return [p1, p2](MemoryControlBlock *mcb) noexcept {
    return p1(mcb) && p2 (mcb);
  };
}

template<MCBPredicate P1, MCBPredicate P2>
MCBPredicate auto operator||(P1 p1, P2 p2) {
  return [p1, p2](MemoryControlBlock *mcb) noexcept {
    return p1(mcb) || p2 (mcb);
  };
}

// template<typename T>
// concept MCBMetric = std::is_nothrow_invocable_r_v<std::size_t, T, const MemoryControlBlock*>;

class FreeMemoryControlBlockList {
  private:
    MemoryControlBlock *first_;

  public:
    FreeMemoryControlBlockList() noexcept;

    // first returns pointer to first block in chain of free blocks.
    // If it is nullptr then the chain is empty
    [[nodiscard]] MemoryControlBlock* first() const noexcept;

    // setFirst sets first free block to given one.
    void setFirst(MemoryControlBlock *) noexcept;

    // prepend prepends given block to start of chain of free blocks
    void prepend(MemoryControlBlock *) noexcept;

    // popFree extracts given block from chain of free blocks
    // and moves firstFree if needed
    void pop(MemoryControlBlock *) noexcept;

    // findPred returns first block in chain of free blocks
    // for which pred returns true
    // It returns nullptr if there is no such block
    template<MCBPredicate P>
    MemoryControlBlock* findPred(P pred) const noexcept {
      for (auto *mcb = this->first(); mcb != nullptr; mcb = mcb->nextFree()) {
        if (pred(mcb)) {
          return mcb;
        }
      }
      return nullptr;
    }

}; // MemoryControlBlockList

} // namespace hse::memory

#endif // MEMORY_CONTROL_BLOCK_LIST_H
