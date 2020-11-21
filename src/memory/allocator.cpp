#include "allocator.h"
#include "math/math.h"
#include "memory_control_block.h"
#include "system/system.h"

#ifndef HSE_MALLOC_NO_RANDOM
#include "random/random.h"
#include <random>
#endif

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace hse::memory {

#ifndef HSE_MALLOC_NO_RANDOM
sc69069_t Allocator::randomGenerator(std::random_device
#ifdef HAVE_DEV_URANDOM
                                                  ("/dev/urandom")
#else
                                                  {}
#endif
                                                    ());
#endif

constexpr MCBPredicate auto mcbFits(std::size_t size) {
    return [size](const MemoryControlBlock *mcb) noexcept {
        return mcb->fits(size);
    };
}

std::uintptr_t Allocator::alloc(std::size_t size) {
    return MemoryControlBlock::data(this->allocBlock(size));
}

std::uintptr_t Allocator::alloc(std::size_t size, std::size_t alignment) {
    return MemoryControlBlock::data(this->allocBlock(size, alignment));
}

MemoryControlBlock *Allocator::allocBlock(std::size_t size) {
    auto *mcb = this->freeBlocks.findPred(mcbFits(size));
    if (mcb == nullptr) {
        mcb = this->allocChunk(size);
    }

#ifndef HSE_MALLOC_NO_RANDOM
    // if there is a space to prepend padding block
    if (mcb->fits(2 + MemoryControlBlock::spaceNeeded(size))) {
//        mcb = this->split(mcb, uniform_int_distribution<std::size_t>
//                         (1, mcb->size() - MemoryControlBlock::spaceNeeded(size))
//                         (Allocator::randomGenerator));
        // TODO: SEGFAULT if replace with:
         mcb = this->shiftForward(mcb, uniform_int_distribution<std::size_t>
                          (0, mcb->size() - size)
                          (Allocator::randomGenerator));
    }
#endif

    // split block to not waste space
    this->split(mcb, size);
    this->freeBlocks.pop(mcb);
    mcb->markBusy();
    return mcb;
}

MemoryControlBlock* Allocator::allocBlock(std::size_t size, std::size_t alignment) {
    auto *mcb = this->findFitDataAligned(size, alignment);
    if (mcb == nullptr) {
        mcb = this->allocChunk(size + alignment);

        auto data = MemoryControlBlock::data(mcb);
        auto shift = math::roundUp(data, alignment) - data;
        mcb = this->shiftForward(mcb, shift);
    }

#ifndef HSE_MALLOC_NO_RANDOM
    if (mcb->fits(math::roundUp(sizeof(MemoryControlBlock), alignment) + size)) {
        auto timesAlignment = uniform_int_distribution<std::size_t>
            (0, (mcb->size() - size) / alignment)
            (Allocator::randomGenerator);
        mcb = this->shiftForward(mcb, timesAlignment * alignment);
    }
#endif

    mcb->markBusy();
    this->split(mcb, size);
    this->freeBlocks.pop(mcb);
    return mcb;
}

std::uintptr_t Allocator::realloc(std::uintptr_t ptr, std::size_t size) {
    // realloc(nullptr, size) is equal to alloc(size)
    if (ptr == reinterpret_cast<std::uintptr_t>(nullptr)) {
        return this->alloc(size);
    }
    return MemoryControlBlock::data(this->reallocBlock(MemoryControlBlock::fromDataPtr(ptr), size));
}

MemoryControlBlock *Allocator::reallocBlock(MemoryControlBlock *mcb, std::size_t size) {
    if (mcb->fits(size)) {
        this->split(mcb, size);
        this->freeBlocks.pop(mcb);
        mcb->markBusy();
        return mcb;
    }

    // check if we can absorb next block and there will be enough space
    if (auto *next = MemoryControlBlock::next(mcb);
        !next->busy() && size <= mcb->size() + sizeof(MemoryControlBlock) + next->size()) {
        this->freeBlocks.pop(next);
        MemoryControlBlock::absorbNext(mcb);
        this->split(mcb, size); // mcb can be larger than we need after the absorption
        return mcb;
    }

    auto *oldMCB = mcb;
    mcb = this->allocBlock(size);
    std::copy(reinterpret_cast<std::uint16_t *>(MemoryControlBlock::data(oldMCB)),
        reinterpret_cast<std::uint16_t *>(MemoryControlBlock::data(oldMCB) + oldMCB->size()),
        reinterpret_cast<std::uint16_t *>(MemoryControlBlock::data(mcb)));
    this->freeBlock(oldMCB);
    return mcb;
}

MemoryControlBlock *Allocator::allocChunk(std::size_t size) {
    std::size_t spaceForEnd = MemoryControlBlock::spaceNeeded(0);
    std::size_t spaceNeeded = MemoryControlBlock::spaceNeeded(size) + spaceForEnd;
    std::size_t totalSize = math::roundUp(spaceNeeded, system::PAGE_SIZE());

    auto *mcb = reinterpret_cast<MemoryControlBlock *>(system::mmap(totalSize));
    mcb->setSize(totalSize - sizeof(MemoryControlBlock) - spaceForEnd);
    this->freeBlocks.prepend(mcb);

    auto *end = MemoryControlBlock::next(mcb);
    end->setPrev(mcb);
    end->makeEndOfChunk();

    return mcb;
}

MemoryControlBlock* Allocator::findFitDataAligned(std::size_t size, std::size_t alignment) noexcept {
    auto minWasteShift = std::numeric_limits<std::size_t>::max();
    MemoryControlBlock *mcbWithMinWasteShift = nullptr;

    for (auto *mcb = this->freeBlocks.first; mcb != nullptr; mcb = mcb->nextFree()) {
        auto data = MemoryControlBlock::data(mcb);
        auto shift = math::roundUp(data, alignment) - data;

        if (!mcb->fits(shift + size)) {
            continue;
        }

        if (shift == 0) {
            // mcb->data() is already aligned
            return mcb;
        }

        // at this point, current mcb should be shifted
        // to meet the alignment requirement

        if (auto noWasteShift = math::roundUp(data + MemoryControlBlock::spaceNeeded(1), alignment) - data;
            mcb->fits(noWasteShift + size)) {
            return this->shiftForward(mcb, noWasteShift);
        }

        // we have no choice but to waste shift,
        // so we want to find the least shift possible
        if (shift < minWasteShift) {
            minWasteShift = shift;
            mcbWithMinWasteShift = mcb;
        }

        if (auto *prev = mcb->prev(); prev != nullptr && !prev->busy()) {
            // if previous block is free, then we can absorb shift
            break;
        }
    }

    if (mcbWithMinWasteShift == nullptr) {
        // there was no block thats fits aligned block with given size
        return nullptr;
    }

    return this->shiftForward(mcbWithMinWasteShift, minWasteShift);
}

MemoryControlBlock* Allocator::shiftForward(MemoryControlBlock *mcb, std::size_t shift) noexcept {
    if (shift == 0) {
        return mcb;
    }

    if (shift >= MemoryControlBlock::spaceNeeded(1)) {
        auto *right = this->split(mcb, shift - sizeof(MemoryControlBlock));

        if (auto *prev = mcb->prev(); prev != nullptr && !prev->busy()) {
            this->freeBlocks.pop(mcb);
            MemoryControlBlock::absorbNext(prev);
        }

        return right;
    }

    auto storedMCB = *mcb;
    auto *shiftedMCB = reinterpret_cast<MemoryControlBlock*>(
           reinterpret_cast<std::uint8_t*>(mcb) + shift);

    if (this->freeBlocks.first == mcb) {
        this->freeBlocks.first = shiftedMCB;
    }

    MemoryControlBlock::next(mcb)->setPrev(shiftedMCB);
    if (auto *prev = mcb->prev(); prev != nullptr) {
        mcb->prev()->grow(shift);
    }
    shiftedMCB->setPrevFree(storedMCB.prevFree());
    shiftedMCB->setNextFree(storedMCB.nextFree());
    shiftedMCB->setSize(storedMCB.size() - shift);
    shiftedMCB->setBusy(storedMCB.busy());

    return shiftedMCB;
}

MemoryControlBlock* Allocator::split(MemoryControlBlock *mcb, std::size_t size) noexcept {
    auto *right = MemoryControlBlock::split(mcb, size);
    if (auto *next = MemoryControlBlock::next(right); !next->busy()) {
        this->freeBlocks.pop(next);
        MemoryControlBlock::absorbNext(right);
    }
    return right;
}

void Allocator::free(std::uintptr_t ptr) {
    this->freeBlock(MemoryControlBlock::fromDataPtr(ptr));
}

void Allocator::freeBlock(MemoryControlBlock *mcb) {
    mcb->markFree();
    this->freeBlocks.prepend(mcb);

    if (MemoryControlBlock *next = MemoryControlBlock::next(mcb); !next->busy()) {
        this->freeBlocks.pop(next);
        MemoryControlBlock::absorbNext(mcb);
    }

    if (MemoryControlBlock *prev = mcb->prev(); prev != nullptr && !prev->busy()) {
        this->freeBlocks.pop(mcb);
        MemoryControlBlock::absorbNext(prev);
        mcb = prev;
    }
    this->tryUnmap(mcb);
}

void Allocator::tryUnmap(MemoryControlBlock *mcb) {
    std::uintptr_t from = mcb->prev() == nullptr
        // first block in chunk can be not page-aligned
        ? math::roundDown(reinterpret_cast<std::uintptr_t>(mcb), system::PAGE_SIZE())
        // we do not want to corrupt previous blocks in this page
        : math::roundUp(MemoryControlBlock::data(mcb), system::PAGE_SIZE());

    auto *next = MemoryControlBlock::next(mcb);
    std::uintptr_t to = next->endOfChunk()
        // end of chunk can be not page-aligned
        ? math::roundUp(MemoryControlBlock::data(next), system::PAGE_SIZE())
        // we do not want to corrunt next blocks in this page
        : math::roundDown(reinterpret_cast<std::uintptr_t>(next), system::PAGE_SIZE());

    if (to < from + system::PAGE_SIZE()) {
        // there is no free pages to delete
        return;
    }

    // store block in case it is in pages we are to delete
    auto mcbStored = *mcb;

    system::munmap(from, to - from);

    if (std::ptrdiff_t diff = from - MemoryControlBlock::data(mcb); diff >= 0) {
        // mcb was not first in chunk since moved foreward
        if (diff >= static_cast<std::ptrdiff_t>(MemoryControlBlock::spaceNeeded(1))) {
            // there is enough space for non-empty block
            mcb->setSize(diff - MemoryControlBlock::spaceNeeded(0));
            auto *end = MemoryControlBlock::next(mcb);
            end->setPrev(mcb);
            end->makeEndOfChunk();
        } else {
            // there is no space in current page
            mcb->makeEndOfChunk();
            this->freeBlocks.pop(mcb);
        }
    } else { // block was removed with pages
        if (this->freeBlocks.first == mcb) {
            this->freeBlocks.first = mcbStored.nextFree();
        }
        mcbStored.popFree();
    }

    if (std::ptrdiff_t diff = reinterpret_cast<std::uintptr_t>(next) - to; diff >= 0) {
        // next was not last in chunk since moved backward
        if (diff >= static_cast<std::ptrdiff_t>(MemoryControlBlock::spaceNeeded(1))) {
            // there is enough space for non-empty block
            auto *first = reinterpret_cast<MemoryControlBlock *>(to);
            first->markFree();
            first->setSize(diff - sizeof(MemoryControlBlock) - MemoryControlBlock::spaceNeeded(0));
            first->setPrev(nullptr);
            next->setPrev(first);
            this->freeBlocks.prepend(first);
        } else {
            // there is no space before next
            next->setPrev(nullptr);
        }
    }
}

} // namespace hse::memory
