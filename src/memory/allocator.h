#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "memory_control_block.h"
#include "memory_control_block_list.h"
#include "random/random.h"

#include <cstddef>
#include <cstdint>

namespace hse::memory {
// Allocator is responsible for managing allocated memory pages and chunks of
// blocks
class Allocator {
  private:
    // PRNG generator which is used in every call to random value
    static sc69069_t randomGenerator;

    FreeMemoryControlBlockList freeBlocks;

    // allocBlock returns block from chain of free blocks
    // or allocates memory for new one if needed
    MemoryControlBlock *allocBlock(std::size_t);

    // allocChunk allocates memory pages for new block with given size
    MemoryControlBlock *allocChunk(std::size_t);

    // realloc(mcb, size) tries to enlarge size of given mcb to given size.
    // If size is less than or equal to current size of mcb,
    // then it shrinks it.
    // If there is not enough room to enlarge size of given mcb,
    // it allocates new MemoryControlBlock, copies the data from given mcb,
    // frees the old mcb and returns a pointer to allocated MemoryControlBlock.
    MemoryControlBlock *realloc(MemoryControlBlock *, std::size_t);

    // freeBlock releases block back to chain of blocks,
    // merges it with its neighbors and unmaps free memory pages
    // within given block
    void freeBlock(MemoryControlBlock *);

    // tryUnmap tries to unmap memory pages within given block
    void tryUnmap(MemoryControlBlock *);

    // returns the random shift for given MCB to store provided size and to be
    // alinged from min possible value to max possible value returns zero if it
    // is not possible
    std::size_t randomShift(MemoryControlBlock *, std::size_t, std::size_t) const noexcept;

  public:
    // alloc(size) allocates memory size bytes and returns a pointer to the
    // allocated memory
    std::uintptr_t alloc(std::size_t);

    // realloc(ptr, size) reallocates memory pointed by ptr for given size and
    // returns ptr. If size is less than or equal to current size of allocated
    // memory pointer by ptr, then it shrinks it. If there is not enough room to
    // enlarge memory allocation pointed by ptr, it allocates new allocation,
    // copies the old data pointed to by ptr, frees the old allocation and
    // returns a pointer to allocated memory.
    std::uintptr_t realloc(std::uintptr_t, std::size_t);

    // aligned_alloc(aligment, size) allocates memory size bytes with specified
    // aligment and returns a pointer to the allocated memory
    std::uintptr_t aligned_alloc(std::size_t, std::size_t);

    // free deallocates memory pointed by given pointer
    void free(std::uintptr_t);
};
} // namespace hse::memory

#endif // ALLOCATOR_H
