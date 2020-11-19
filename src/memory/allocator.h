#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "memory_control_block.h"
#include "memory_control_block_list.h"

#ifndef HSE_MALLOC_NO_RANDOM
#include "random/random.h"
#endif

#include <cstddef>
#include <cstdint>


namespace hse::memory {
// Allocator is responsible for managing allocated memory pages and chunks of
// blocks
class Allocator {
  private:

#ifndef HSE_MALLOC_NO_RANDOM
    // PRNG generator which is used in every call to random value
    static sc69069_t randomGenerator;
#endif

    FreeMemoryControlBlockList freeBlocks;

    // allocBlock returns block with given size from chain of free blocks
    // or allocates memory for new one if needed.
    MemoryControlBlock* allocBlock(std::size_t size);

    // allockBlock(size, alignment) is the same as allocBlock(size),
    // but returned block data is aligned by given alignment
    MemoryControlBlock* allocBlock(std::size_t size, std::size_t alignment);

    // realloc(mcb, size) tries to enlarge size of given mcb to given size.
    // If size is less than or equal to current size of mcb,
    // then it shrinks it.
    // If there is not enough room to enlarge size of given mcb,
    // it allocates new MemoryControlBlock, copies the data from given mcb,
    // frees the old mcb and returns a pointer to allocated MemoryControlBlock.
    MemoryControlBlock *reallocBlock(MemoryControlBlock *, std::size_t);

    // freeBlock releases block back to chain of blocks,
    // merges it with its neighbors and unmaps free memory pages
    // within given block
    void freeBlock(MemoryControlBlock *);

    // allocChunk allocates memory pages for new block with given size
    [[nodiscard]] MemoryControlBlock *allocChunk(std::size_t size);

    // findFitDataAligned returns block which fits given size
    // and data is aligned by given alignment.
    // It returns nullptr if there is no such block
    MemoryControlBlock *findFitDataAligned(std::size_t size, std::size_t alignment) noexcept;

    // shiftForward shifts given MCB forward
    // and prepends it with another non-empty MCB if possible.
    // It returns pointer to shifted one.
    // Following conditions should be met:
    // 1. mcb should not be busy
    // 2. shift should be multiple of 2
    // 3. mcb->fits(shift + 2)
    [[nodiscard]] MemoryControlBlock* shiftForward(MemoryControlBlock* mcb, std::size_t shift) noexcept;

    // split splits the block into two block
    // and tries to absorb the block next to the right one
    MemoryControlBlock* split(MemoryControlBlock *mcb, std::size_t size) noexcept;

    // tryUnmap tries to unmap memory pages within given block
    void tryUnmap(MemoryControlBlock *);

  public:
    // alloc(size) allocates memory size bytes and returns a pointer to the
    // allocated memory
    [[nodiscard]] std::uintptr_t alloc(std::size_t);

    // alloc(size, alignment) allocates memory of size bytes with specified
    // aligment and returns a pointer to the allocated memory
    [[nodiscard]] std::uintptr_t alloc(std::size_t size, std::size_t alignment);

    // realloc(ptr, size) reallocates memory pointed by ptr for given size and
    // returns ptr. If size is less than or equal to current size of allocated
    // memory pointer by ptr, then it shrinks it. If there is not enough room to
    // enlarge memory allocation pointed by ptr, it allocates new allocation,
    // copies the old data pointed to by ptr, frees the old allocation and
    // returns a pointer to allocated memory.
    [[nodiscard]] std::uintptr_t realloc(std::uintptr_t, std::size_t);

    // free deallocates memory pointed by given pointer
    void free(std::uintptr_t);
};
} // namespace hse::memory

#endif // ALLOCATOR_H
