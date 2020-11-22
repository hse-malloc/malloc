#include "memory_control_block.h"
#include "math/math.h"

#include <cstddef>
#include <cstdint>
#include <utility>

namespace hse::memory {

MemoryControlBlock *MemoryControlBlock::fromDataPtr(std::uintptr_t ptr) noexcept {
    return reinterpret_cast<MemoryControlBlock *>(ptr) - 1;
}

std::uintptr_t MemoryControlBlock::data(const MemoryControlBlock *mcb) noexcept {
    return reinterpret_cast<std::uintptr_t>(mcb + 1);
}

std::size_t MemoryControlBlock::size() const noexcept {
    return math::roundDown(this->size_, 2);
}

bool MemoryControlBlock::empty() const noexcept { return this->size() == 0; }

void MemoryControlBlock::setSize(std::size_t size) noexcept {
    this->size_ = math::roundUp(size, 2) | static_cast<std::size_t>(this->busy());
}

void MemoryControlBlock::grow(std::size_t size) noexcept {
    return this->setSize(this->size() + size);
}

bool MemoryControlBlock::fits(std::size_t size) const noexcept {
    return this->size() >= size;
}

bool MemoryControlBlock::busy() const noexcept {
    return math::nthBit(this->size_, 0);
}

void MemoryControlBlock::setBusy(bool busy) noexcept {
    if (busy) {
        this->markBusy();
    } else {
        this->markFree();
    }
}

void MemoryControlBlock::markBusy() noexcept {
    this->size_ = math::setNthBit(this->size_, 0);
}

void MemoryControlBlock::markFree() noexcept {
    this->size_ = math::clearNthBit(this->size_, 0);
}

MemoryControlBlock *MemoryControlBlock::prev() const noexcept {
    return this->prev_;
}

void MemoryControlBlock::setPrev(MemoryControlBlock *prev) noexcept {
    this->prev_ = prev;
}

MemoryControlBlock *MemoryControlBlock::next(const MemoryControlBlock *mcb) noexcept {
    return reinterpret_cast<MemoryControlBlock *>(MemoryControlBlock::data(mcb) + mcb->size());
}

MemoryControlBlock* MemoryControlBlock::prevFree() const noexcept {
    return this->prevFree_;
}

void MemoryControlBlock::setPrevFree(MemoryControlBlock *mcb, MemoryControlBlock *prev) noexcept {
    if ((mcb->prevFree_ = prev) == nullptr) {
        return;
    }
    prev->nextFree_ = mcb;
}

MemoryControlBlock *MemoryControlBlock::nextFree() const noexcept {
    return this->nextFree_;
}

void MemoryControlBlock::setNextFree(MemoryControlBlock *mcb, MemoryControlBlock *next) noexcept {
    if ((mcb->nextFree_ = next) == nullptr) {
        return;
    }
    next->prevFree_ = mcb;
}

} // namespace hse::memory
