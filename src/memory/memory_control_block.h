#ifndef MEMORY_CONTROL_BLOCK_H
#define MEMORY_CONTROL_BLOCK_H

#include <cstddef>
#include <cstdint>

namespace hse::memory {
// MemoryControlBlock is placed right before
// every allocated block of memory and describes following block
class MemoryControlBlock {
  private:
    // size_ holds availability of block in first least bit
    // and size of block in rest. Thus, size of block is always 2-aligned
    std::size_t size_;

    // prev_ holds pointer to previous block in same chunk.
    // It is nullptr if there is no previous block, which means
    // that this block is first in this chunk
    MemoryControlBlock *prev_;

    // prevFree_ holds a pointer to next free block in chain of free blocks.
    // It should be nullptr if current block is busy.
    // If it is nullptr then there is no next free block and this block is
    // the last in chain of free blocks
    MemoryControlBlock *prevFree_;

    // prevFree_ holds a pointer to previuos free block in chain of free blocks.
    // It should be nullptr if current block is busy.
    // If it is nullptr then there is no previous free block and this block is
    // the first in chain of free blocks
    MemoryControlBlock *nextFree_;

    // setPrevFree sets previous free block in chain of free blocks
    // and sets its next free block to current
    void setPrevFree(MemoryControlBlock *) noexcept;

  public:
    MemoryControlBlock(std::size_t size = 0, MemoryControlBlock *prev = nullptr,
                       MemoryControlBlock *prevFree = nullptr,
                       MemoryControlBlock *nextFree = nullptr);

    static MemoryControlBlock *fromPtr(std::uintptr_t) noexcept;

    // spaceNeeded returns how many bytes is needed for block with given size
    static std::size_t spaceNeeded(std::size_t) noexcept;

    // data returns a pointer to data which this block holds
    std::uintptr_t data() const noexcept;

    // size returns the size of data in bytes
    std::size_t size() const noexcept;

    // empty returns if there is zero space for data
    bool empty() const noexcept;

    // setSize sets size of block
    std::size_t setSize(std::size_t) noexcept;

    // grow increases size of block by given value
    std::size_t grow(std::size_t) noexcept;

    // fits returns if there is enough space in block for given size
    bool fits(std::size_t) const noexcept;

    // split tries to split the block into two blocks,
    // where first of them has given size.
    // It returns pointer to next block in case of split,
    // or pointer to itself otherwise
    MemoryControlBlock *split(std::size_t) noexcept;

    // busy returns if the block is marked as busy
    bool busy() const noexcept;

    // setBusy marks block as busy
    void setBusy() noexcept;

    // setFree marks block as free
    void setFree() noexcept;

    // prev returns a pointer to previous block in same chunk.
    // If it is nullptr then it is the first block in chunk.
    MemoryControlBlock *prev() const noexcept;

    // setPrev sets the previous block in same chunk
    void setPrev(MemoryControlBlock *) noexcept;

    // next returns a pointer to next block in same chunk.
    // NOTE: it is always a non-nullptr pointer and we need
    // to check if it is a valid pointer before dereferencing it
    MemoryControlBlock *next() const noexcept;

    // absorbNext removes next block from chain of free blocks
    // and absorbs it
    void absorbNext() noexcept;

    // nextFree returns a pointer to next free block in chain of free blocks
    // If it is nullptr then this is the last block in the chain.
    MemoryControlBlock *nextFree() const noexcept;

    // setNextFree sets next free block in chain of free blocks
    // and sets its previous free block to current
    void setNextFree(MemoryControlBlock *) noexcept;

    // popFree extracts current block from chain of free blocks
    // and sets its previuos and next free blocks to nullptr
    void popFree() noexcept;

    // first returns if the block is the first in chunk
    bool firstInChunk() const noexcept;

    // makeFirstInChunk marks block as first in chunk
    void makeFirstInChunk() noexcept;

    // endOfChunk returns if block denotes the end of chunk
    bool endOfChunk() const noexcept;

    // makeEndOfChunk marks block as end of chunk
    void makeEndOfChunk() noexcept;
};
} // namespace hse::memory

#endif // MEMORY_CONTROL_BLOCK_H
