#ifndef MEMORY_CONTROL_BLOCK_LIST_H
#define MEMORY_CONTROL_BLOCK_LIST_H

#include "memory_control_block.h"

namespace hse::memory {

class FreeMemoryControlBlockList {
  public:
    MemoryControlBlock *first;

    FreeMemoryControlBlockList();

    // prependFree prepends given block to start of chain of free blocks
    void prepend(MemoryControlBlock *) noexcept;

    // popFree extracts given block from chain of free blocks
    // and moves firstFree if needed
    void pop(MemoryControlBlock *) noexcept;

    // findIf returns first block in chain of free blocks
    // for which pred returns true
    // It returns nullptr if there is no such block
    template<typename UnaryPredicate>
    MemoryControlBlock* findIf(UnaryPredicate pred) const noexcept {
      MemoryControlBlock *mcb;
      for (mcb = this->first; mcb != nullptr && !pred(mcb); mcb = mcb->nextFree()) {}
      return mcb;
    }

    // MemoryControlBlock* searchFit(std::size_t size) const noexcept;
}; // MemoryControlBlockList

} // namespace hse::memory

#endif // MEMORY_CONTROL_BLOCK_LIST_H
