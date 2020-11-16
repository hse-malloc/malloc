#include "memory_control_block_list.h"
#include "memory_control_block.h"

namespace hse::memory {

FreeMemoryControlBlockList::FreeMemoryControlBlockList() noexcept :
    first(nullptr) {}

void FreeMemoryControlBlockList::prepend(MemoryControlBlock *mcb) noexcept {
   mcb->setNextFree(this->first);
   this->first = mcb;
}

void FreeMemoryControlBlockList::pop(MemoryControlBlock *mcb) noexcept {
    if (this->first == mcb) {
        this->first = mcb->nextFree();
    }
    mcb->popFree();
}

} // namespace hse::memory
