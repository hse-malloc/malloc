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

constexpr MCBPredicate auto mcbFits(std::size_t);
constexpr MCBPredicate auto mcbFitsAlignedData(std::size_t size, std::size_t alignment);

// Allocator is responsible for managing allocated memory pages and chunks of
// blocks
class Allocator {
  private:

#ifndef HSE_MALLOC_NO_RANDOM
    // PRNG generator which is used in every call to random value
    static sc69069_t randomGenerator;
#endif

    FreeMemoryControlBlockList freeBlocks;

    // allockBlock returns block with given size and data with given alignment
    // from chain of free blocks or allocates memory for new one if needed. 
    [[nodiscard]] MemoryControlBlock* allocBlock(std::size_t size, std::size_t alignment);

    // realloc(mcb, size) tries to enlarge size of given mcb to given size.
    // If size is less than or equal to current size of mcb,
    // then it shrinks it.
    // If there is not enough room to enlarge size of given mcb,
    // it allocates new MemoryControlBlock, copies the data from given mcb,
    // frees the old mcb and returns a pointer to allocated MemoryControlBlock.
    [[nodiscard]] MemoryControlBlock *reallocBlock(MemoryControlBlock *, std::size_t);

    // freeBlock releases block back to chain of blocks,
    // merges it with its neighbors and unmaps free memory pages
    // within given block
    void freeBlock(MemoryControlBlock *);

    // allocChunk allocates memory pages for new block with given size
    [[nodiscard]] MemoryControlBlock *allocChunk(std::size_t size);

    // shiftForward shifts given MCB forward
    // and prepends it with another non-empty MCB if possible.
    // It returns pointer to shifted one.
    // Following conditions should be met:
    // 1. mcb should not be busy
    // 2. shift should be multiple of 2
    // 3. mcb->fits(shift + 2)
    [[nodiscard]] MemoryControlBlock* shiftForward(MemoryControlBlock* mcb, std::size_t shift) noexcept;

    // split tries to split given block into two blocks,
    // where first of them has given size.
    // It prepends right block to chain of free blocks
    // and tries to absorb the block next to the right one.
    // It returns pointer to second block in case of split
    // or pointer to given block otherwise.
    // NOTE: size should be a multiple of 2
    MemoryControlBlock* split(MemoryControlBlock *mcb, std::size_t size) noexcept;

    // absorbNext removes block next to given from chain of free blocks
    // and absorbs it
    void absorbNext(MemoryControlBlock *mcb) noexcept;

    // tryUnmap tries to unmap memory pages within given block
    void tryUnmap(MemoryControlBlock *);

    static constexpr MCBPredicate auto mcbFitsAlignedData(std::size_t size, std::size_t alignment);
    // shiftToAlignData returns how many bytes mcb should be shifted right
    // to make its data aligned with given alignmebt
    static std::size_t shiftToAlignData(const MemoryControlBlock *mcb, std::size_t alignment) noexcept;
  public:
    // alloc(size, alignment) allocates memory of size bytes with specified
    // aligment and returns a pointer to the allocated memory.
    // NOTE: size should be a multiple of max(2, alignment)
    [[nodiscard]] std::uintptr_t alloc(std::size_t size, std::size_t alignment);

    // realloc(ptr, size) reallocates memory pointed by ptr for given size and
    // returns ptr. If size is less than or equal to current size of allocated
    // memory pointer by ptr, then it shrinks it. If there is not enough room to
    // enlarge memory allocation pointed by ptr, it allocates new allocation,
    // copies the old data pointed to by ptr, frees the old allocation and
    // returns a pointer to allocated memory.
    // NOTE: size should be a multiple of 2
    [[nodiscard]] std::uintptr_t realloc(std::uintptr_t, std::size_t);

    // free deallocates memory pointed by given pointer
    void free(std::uintptr_t);
};
} // namespace hse::memory

#endif // ALLOCATOR_H
