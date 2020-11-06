#include "allocator.h"
#include "math/math.h"
#include "memory_control_block.h"
#include "random/random.h"
#include "system/system.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <random>

namespace hse::memory {

thread_local sc69069_t Allocator::randomGenerator(std::random_device("/dev/urandom")());


void Allocator::prependFree(MemoryControlBlock *mcb) noexcept {
    mcb->setNextFree(this->firstFree);
    this->firstFree = mcb;
}

MemoryControlBlock *Allocator::searchFit(std::size_t size) const noexcept {
    MemoryControlBlock *mcb;
    for (mcb = this->firstFree; (mcb != nullptr) && !mcb->fits(size);
         mcb = mcb->nextFree())
        ;
    return mcb;
}

void Allocator::popFree(MemoryControlBlock *mcb) noexcept {
    if (this->firstFree == mcb)
        this->firstFree = mcb->nextFree();
    mcb->popFree();
}

std::uintptr_t Allocator::alloc(std::size_t size) {
    return this->allocBlock(size)->data();
}

MemoryControlBlock *Allocator::allocBlock(std::size_t size) {
    MemoryControlBlock *mcb = this->searchFit(size);
    if (mcb == nullptr)
        mcb = this->allocChunk(size);

    // if there is a space to prepend padding block
    if (mcb->fits(2 + MemoryControlBlock::spaceNeeded(size))) {
        mcb = mcb->split(uniform_int_distribution<std::size_t>(
            1, mcb->size() - MemoryControlBlock::spaceNeeded(size))(
            Allocator::randomGenerator));
    }

    // split block to not waste space
    mcb->split(size);

    this->popFree(mcb);
    mcb->setBusy();
    return mcb;
}

MemoryControlBlock *Allocator::allocChunk(std::size_t size) {
    std::size_t spaceForEnd = MemoryControlBlock::spaceNeeded(0);
    std::size_t spaceNeeded =
        MemoryControlBlock::spaceNeeded(size) + spaceForEnd;
    std::size_t totalSize = math::roundUp(spaceNeeded, system::PAGE_SIZE());

    auto *mcb = reinterpret_cast<MemoryControlBlock *>(system::mmap(totalSize));
    mcb->setSize(totalSize - sizeof(MemoryControlBlock) - spaceForEnd);
    this->prependFree(mcb);
    MemoryControlBlock *end = mcb->next();
    end->setPrev(mcb);
    end->makeEndOfChunk();
    return mcb;
}

std::uintptr_t Allocator::realloc(std::uintptr_t ptr, std::size_t size) {
    // realloc(nullptr, size) is equal to alloc(size)
    if (ptr == reinterpret_cast<std::uintptr_t>(nullptr))
        return this->alloc(size);
    return this->realloc(MemoryControlBlock::fromPtr(ptr), size)->data();
}

void Allocator::free(std::uintptr_t ptr) {
    this->freeBlock(MemoryControlBlock::fromPtr(ptr));
}

void Allocator::freeBlock(MemoryControlBlock *mcb) {
    mcb->setFree();
    this->prependFree(mcb);

    if (MemoryControlBlock *next = mcb->next(); next && !next->busy()) {
        if (this->firstFree == next)
            this->firstFree = next->nextFree();
        mcb->absorbNext();
    }

    if (MemoryControlBlock *prev = mcb->prev(); prev && !prev->busy()) {
        if (this->firstFree == mcb)
            this->firstFree = mcb->nextFree();
        prev->absorbNext();
        mcb = prev;
    }
    this->tryUnmap(mcb);
}

void Allocator::tryUnmap(MemoryControlBlock *mcb) {
    std::uintptr_t from;
    if (mcb->firstInChunk())
        // first block in chunk can be not page-aligned
        from = math::roundDown(reinterpret_cast<std::uintptr_t>(mcb),
                               system::PAGE_SIZE());
    else
        // we do not want to corrupt previous blocks in this page
        from = math::roundUp(mcb->data(), system::PAGE_SIZE());

    auto *next = mcb->next();
    std::uintptr_t to;
    if (next->endOfChunk())
        // end of chunk can be not page-aligned
        to = math::roundUp(next->data(), system::PAGE_SIZE());
    else
        // we do not want to corrunt next blocks in this page
        to = math::roundDown(reinterpret_cast<std::uintptr_t>(next),
                             system::PAGE_SIZE());

    if (to < from + system::PAGE_SIZE())
        // there is no free pages to delete
        return;

    // store block in case it is in pages we are to delete
    MemoryControlBlock mcbStored = *mcb;

    system::munmap(from, to - from);

    if (std::ptrdiff_t diff = from - mcb->data(); diff >= 0) {
        // mcb was not first in chunk since moved foreward
        if (diff >=
            static_cast<std::ptrdiff_t>(MemoryControlBlock::spaceNeeded(1))) {
            // there is enough space for non-empty block
            mcb->setSize(diff - MemoryControlBlock::spaceNeeded(0));
            MemoryControlBlock *end = mcb->next();
            end->setPrev(mcb);
            end->makeEndOfChunk();
        } else {
            // there is no space in current page
            mcb->makeEndOfChunk();
            this->popFree(mcb);
        }
    } else {
        // block was removed with pages
        if (this->firstFree == mcb)
            this->firstFree = mcbStored.nextFree();
        mcbStored.popFree();
    }

    if (std::ptrdiff_t diff = reinterpret_cast<std::uintptr_t>(next) - to;
        diff >= 0) {
        // next was not last in chunk since moved backward
        if (diff >=
            static_cast<std::ptrdiff_t>(MemoryControlBlock::spaceNeeded(1))) {
            // there is enough space for non-empty block
            auto *first = reinterpret_cast<MemoryControlBlock *>(to);
            first->setFree();
            first->setSize(diff - sizeof(MemoryControlBlock) -
                           MemoryControlBlock::spaceNeeded(0));
            first->makeFirstInChunk();
            next->setPrev(first);
            this->prependFree(first);
        } else
            // there is no space before next
            next->makeFirstInChunk();
    }
}

MemoryControlBlock *Allocator::realloc(MemoryControlBlock *mcb,
                                       std::size_t size) {
    if (mcb->fits(size)) {
        mcb->split(size);
        return mcb;
    }

    // check if there we can absorb next block and there will be enough space
    if (auto *next = mcb->next();
        !next->busy() &&
        size <= mcb->size() + sizeof(MemoryControlBlock) + next->size()) {
        mcb->absorbNext();
        mcb->split(size); // mcb can be larger than we need after the absorption
        return mcb;
    }

    auto *oldMCB = mcb;
    mcb = this->allocBlock(size);
    std::copy(
        reinterpret_cast<unsigned char *>(oldMCB->data()),
        reinterpret_cast<unsigned char *>(oldMCB->data() + oldMCB->size()),
        mcb);
    this->freeBlock(oldMCB);
    return mcb;
}
} // namespace hse::memory
