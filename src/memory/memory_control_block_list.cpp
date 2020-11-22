#include "memory_control_block_list.h"
#include "memory_control_block.h"

namespace hse::memory {

FreeMemoryControlBlockList::FreeMemoryControlBlockList() noexcept :
    first_(nullptr) {}

MemoryControlBlock *FreeMemoryControlBlockList::first() const noexcept {
    return this->first_;
}

void FreeMemoryControlBlockList::setFirst(MemoryControlBlock *mcb) noexcept {
    if ((this->first_ = mcb) == nullptr) {
        return;
    }
    MemoryControlBlock::setPrevFree(mcb, nullptr);
}

void FreeMemoryControlBlockList::prepend(MemoryControlBlock *mcb) noexcept {
    mcb->markFree();
    MemoryControlBlock::setNextFree(mcb, this->first());
    this->setFirst(mcb);
}

void FreeMemoryControlBlockList::pop(MemoryControlBlock *mcb) noexcept {
    if (this->first() == mcb) {
        this->setFirst(mcb->nextFree());
    }

    if (auto *prev = mcb->prevFree(); prev != nullptr) {
        MemoryControlBlock::setNextFree(prev, mcb->nextFree());
    } else if (auto *next = mcb->nextFree(); next != nullptr) {
        MemoryControlBlock::setPrevFree(next, prev);
    }
    MemoryControlBlock::setPrevFree(mcb, nullptr);
    MemoryControlBlock::setNextFree(mcb, nullptr);
}

} // namespace hse::memory
