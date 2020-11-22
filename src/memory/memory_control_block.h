#ifndef MEMORY_CONTROL_BLOCK_H
#define MEMORY_CONTROL_BLOCK_H

#include <cstddef>
#include <cstdint>
#include <tuple>

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
  public:
    // fromDataPtr returns pointer to block which controls data pointed by given ptr
    static MemoryControlBlock *fromDataPtr(std::uintptr_t) noexcept;

    // data returns a pointer to data which this block holds
    [[nodiscard]] static std::uintptr_t data(const MemoryControlBlock *mcb) noexcept;

    // size returns the size of data in bytes
    [[nodiscard]] std::size_t size() const noexcept;

    // empty returns if there is zero space for data
    [[nodiscard]] bool empty() const noexcept;

    // setSize sets size of block.
    // NOTE: size should be multiple of 2
    void setSize(std::size_t) noexcept;

    // grow increases size of block by given value
    // NOTE: size should be multiple of 2
    void grow(std::size_t) noexcept;

    // fits returns if there is enough space in block for given size
    [[nodiscard]] bool fits(std::size_t) const noexcept;

    // split tries to split the block into two blocks,
    // where first of them has given size.
    // It returns pointer to second block in case of split,
    // or pointer to itself otherwise
    [[nodiscard]] static MemoryControlBlock* split(MemoryControlBlock *mcb, std::size_t);

    // busy returns if the block is marked as busy
    [[nodiscard]] bool busy() const noexcept;

    // setBusy sets busy bit
    void setBusy(bool) noexcept;

    // markBusy marks block as busy
    void markBusy() noexcept;

    // markFree marks block as free
    void markFree() noexcept;

    // prev returns a pointer to previous block in same chunk.
    // If it is nullptr then it is the first block in chunk.
    [[nodiscard]] MemoryControlBlock *prev() const noexcept;

    // setPrev sets the previous block in same chunk
    void setPrev(MemoryControlBlock *) noexcept;

    // next returns a pointer to next block in same chunk.
    // NOTE: it is always a non-nullptr pointer and we need
    // to check if it is a valid pointer before dereferencing it
    [[nodiscard]] static MemoryControlBlock *next(const MemoryControlBlock *mcb) noexcept;

    // prevFree returns a pointer to previous free block in chain of free blocks.
    // If it is nullptr then this is the first block in the chain
    [[nodiscard]] MemoryControlBlock *prevFree() const noexcept;

    // setPrevFree sets previous free block in chain of free blocks
    // and sets its next free block to current
    static void setPrevFree(MemoryControlBlock *mcb, MemoryControlBlock *prev) noexcept;

    // nextFree returns a pointer to next free block in chain of free blocks
    // If it is nullptr then this is the last block in the chain.
    [[nodiscard]] MemoryControlBlock *nextFree() const noexcept;

    // setNextFree sets next free block in chain of free blocks
    // and sets its previous free block to current
    static void setNextFree(MemoryControlBlock *mcb, MemoryControlBlock *next) noexcept;
};

} // namespace hse::memory

#endif // MEMORY_CONTROL_BLOCK_H
