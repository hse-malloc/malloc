#include "memory_control_block.h"
#include "math/math.h"

#include <cstddef>
#include <cstdint>

namespace hse::memory {

MemoryControlBlock::MemoryControlBlock(std::size_t size,
                                       MemoryControlBlock *prev,
                                       MemoryControlBlock *prevFree,
                                       MemoryControlBlock *nextFree)
    : prev_(prev) {
    this->setSize(size);
    this->setPrevFree(prevFree);
    this->setNextFree(nextFree);
}

MemoryControlBlock *MemoryControlBlock::fromDataPtr(std::uintptr_t ptr) noexcept {
    return reinterpret_cast<MemoryControlBlock *>(ptr) - 1;
}

std::size_t MemoryControlBlock::spaceNeeded(std::size_t size) noexcept {
    return sizeof(MemoryControlBlock) + math::roundUp<std::size_t>(size, 2);
}

std::uintptr_t MemoryControlBlock::data() const noexcept {
    return reinterpret_cast<std::uintptr_t>(this + 1);
}

std::size_t MemoryControlBlock::size() const noexcept {
    return math::roundDown<std::size_t>(this->size_, 2);
}

bool MemoryControlBlock::empty() const noexcept { return this->size() == 0; }

std::size_t MemoryControlBlock::setSize(std::size_t size) noexcept {
    return (this->size_ = math::roundUp<std::size_t>(size, 2) | this->busy());
}

std::size_t MemoryControlBlock::grow(std::size_t size) noexcept {
    return this->setSize(this->size() + size);
}

bool MemoryControlBlock::fits(std::size_t size) const noexcept {
    return this->size() >= size;
}

MemoryControlBlock *MemoryControlBlock::split(std::size_t size) noexcept {
    size = math::roundUp<std::size_t>(size, 2);
    if (!this->fits(size + MemoryControlBlock::spaceNeeded(1)))
        return this;

    std::size_t oldSize = this->size();
    this->setSize(size);

    MemoryControlBlock *next = this->next();
    next->setFree();
    next->setSize(oldSize - size - sizeof(MemoryControlBlock));
    next->setPrev(this);
    next->next()->setPrev(next);

    next->setNextFree(this->nextFree());
    next->setPrevFree(this);
    return next;
}

bool MemoryControlBlock::busy() const noexcept {
    return math::nthBit(this->size_, 0);
}

void MemoryControlBlock::setBusy() noexcept {
    this->size_ = math::setNthBit(this->size_, 0);
    this->popFree();
}

void MemoryControlBlock::setFree() noexcept {
    this->size_ = math::clearNthBit(this->size_, 0);
}

MemoryControlBlock *MemoryControlBlock::prev() const noexcept {
    return this->prev_;
}

void MemoryControlBlock::setPrev(MemoryControlBlock *prev) noexcept {
    this->prev_ = prev;
}

MemoryControlBlock *MemoryControlBlock::next() const noexcept {
    return reinterpret_cast<MemoryControlBlock *>(this->data() + this->size());
}

void MemoryControlBlock::absorbNext() noexcept {
    MemoryControlBlock *next = this->next();
    next->popFree();
    next->next()->setPrev(this);
    this->grow(sizeof(MemoryControlBlock) + next->size());
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

bool MemoryControlBlock::firstInChunk() const noexcept {
    return this->prev() == nullptr;
}

void MemoryControlBlock::makeFirstInChunk() noexcept {
    this->setPrev(nullptr);
}

bool MemoryControlBlock::endOfChunk() const noexcept {
    return this->busy() && this->size() == 0;
}

void MemoryControlBlock::makeEndOfChunk() noexcept {
    this->setBusy();
    this->setSize(0);
    this->popFree();
}

bool MemoryControlBlock::isAligned(std::size_t aligment) const noexcept {
    return (aligment == 0 ? false : this->data() % aligment);
}

std::size_t
MemoryControlBlock::minNeededShift(std::size_t alignment) const noexcept {
    return (this->isAligned(alignment)
                ? 0
                : (alignment - (this->data() % alignment)));
}

std::size_t
MemoryControlBlock::maxPossibleShift(std::size_t alignment,
                                     std::size_t needed_size) const noexcept {
    std::size_t max_shift = this->minNeededShift(alignment);
    if (needed_size + max_shift > this->size()) // it is not possible
        return 0;
    std::size_t remainded_space = this->size() - needed_size - max_shift;
    return (remainded_space / alignment) * alignment + max_shift;
}

MemoryControlBlock *
MemoryControlBlock::shiftForward(std::size_t size) noexcept {
    if (this->busy() || this->prev() == nullptr || size == 0)
        return this;

    size = math::roundUp<std::size_t>(size, 2);
    auto *new_mcb = reinterpret_cast<MemoryControlBlock *>(
        reinterpret_cast<std::uintptr_t>(this + size));

    this->prev()->grow(size);
    this->next()->setPrev(new_mcb);

    new_mcb->setPrevFree(this->prevFree_);
    new_mcb->setNextFree(this->nextFree_);
    new_mcb->size_ = this->size_;
    return new_mcb;
}

} // namespace hse::memory
