#include "memory_control_block.h"
#include "math/math.h"

#include <cstddef>
#include <cstdint>
#include <utility>

namespace hse::memory {

MemoryControlBlock *MemoryControlBlock::fromDataPtr(std::uintptr_t ptr) noexcept {
    return reinterpret_cast<MemoryControlBlock *>(ptr) - 1;
}

std::size_t MemoryControlBlock::spaceNeeded(std::size_t size) noexcept {
    return sizeof(MemoryControlBlock) + math::roundUp<std::size_t>(size, 2);
}

std::uintptr_t MemoryControlBlock::data(const MemoryControlBlock *mcb) noexcept {
    return reinterpret_cast<std::uintptr_t>(mcb + 1);
}

std::size_t MemoryControlBlock::size() const noexcept {
    return math::roundDown<std::size_t>(this->size_, 2);
}

bool MemoryControlBlock::empty() const noexcept { return this->size() == 0; }

std::size_t MemoryControlBlock::setSize(std::size_t size) noexcept {
    return (this->size_ = math::roundUp<std::size_t>(size, 2) | static_cast<std::size_t>(this->busy()));
}

std::size_t MemoryControlBlock::grow(std::size_t size) noexcept {
    return this->setSize(this->size() + size);
}

bool MemoryControlBlock::fits(std::size_t size) const noexcept {
    return this->size() >= size;
}

MemoryControlBlock* MemoryControlBlock::split(MemoryControlBlock *mcb, std::size_t size) noexcept {
    if (size == 0) {
        return mcb;
    }
    size = math::roundUp<std::size_t>(size, 2);
    if (!mcb->fits(size + MemoryControlBlock::spaceNeeded(1))) {
        return mcb;
    }

    std::size_t oldSize = mcb->size();
    mcb->setSize(size);

    MemoryControlBlock *right = MemoryControlBlock::next(mcb);
    right->markFree();
    right->setSize(oldSize - size - sizeof(MemoryControlBlock));
    right->setPrev(mcb); 
    MemoryControlBlock::next(right)->setPrev(right);

    right->setNextFree(mcb->nextFree());
    right->setPrevFree(mcb); 
 
    return right;
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
    this->popFree();
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

void MemoryControlBlock::absorbNext(MemoryControlBlock *mcb) noexcept {
    MemoryControlBlock *next = MemoryControlBlock::next(mcb);
    next->popFree();
    MemoryControlBlock::next(next)->setPrev(mcb);
    mcb->grow(sizeof(MemoryControlBlock) + next->size());
}

MemoryControlBlock* MemoryControlBlock::prevFree() const noexcept {
    return this->prevFree_;
}

void MemoryControlBlock::setPrevFree(MemoryControlBlock *prevFree) noexcept {
    if ((this->prevFree_ = prevFree) == nullptr) {
        return;
    }
    prevFree->nextFree_ = this;
}

MemoryControlBlock *MemoryControlBlock::nextFree() const noexcept {
    return this->nextFree_;
}

void MemoryControlBlock::setNextFree(MemoryControlBlock *nextFree) noexcept {
    if ((this->nextFree_ = nextFree) == nullptr) {
        return;
    }
    nextFree->prevFree_ = this;
}

void MemoryControlBlock::popFree() noexcept {
    if (this->prevFree_ != nullptr) {
        this->prevFree_->setNextFree(this->nextFree());
    } else if (auto *nextFree = this->nextFree(); nextFree != nullptr) {
        nextFree->setPrevFree(this->prevFree_);
    }
    this->setPrevFree(nullptr);
    this->setNextFree(nullptr);
}

bool MemoryControlBlock::endOfChunk() const noexcept {
    return this->busy() && this->size() == 0;
}

void MemoryControlBlock::makeEndOfChunk() noexcept {
    this->markBusy();
    this->setSize(0);
    this->popFree();
}

} // namespace hse::memory
