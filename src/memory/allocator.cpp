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

constexpr MCBPredicate auto Allocator::mcbFitsAlignedData(std::size_t size, std::size_t alignment) {
    return [size, alignment](const MemoryControlBlock *mcb) noexcept {
        return mcb->fits(Allocator::shiftToAlignData(mcb, alignment) + size);
    };
}

std::uintptr_t Allocator::alloc(std::size_t size, std::size_t alignment) {
    return MemoryControlBlock::data(this->allocBlock(size, alignment));
}

MemoryControlBlock* Allocator::allocBlock(std::size_t size, std::size_t alignment) {
    auto *mcb = this->freeBlocks.findPred(mcbFitsAlignedData(size, alignment));
    if (mcb == nullptr) {
        mcb = this->allocChunk(size + alignment);
    }

    mcb = this->shiftForward(mcb, Allocator::shiftToAlignData(mcb, alignment));

#ifndef HSE_MALLOC_NO_RANDOM
    // check if there is space to prepend with padding block
    if (mcb->fits(2 + math::roundUp(sizeof(MemoryControlBlock), alignment) + size)) {
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
    if (ptr == reinterpret_cast<std::uintptr_t>(nullptr)) {
        // realloc(nullptr, size) is equal to alloc(size)
        return this->alloc(size, 1);
    }
    return MemoryControlBlock::data(this->reallocBlock(MemoryControlBlock::fromDataPtr(ptr), size));
}

MemoryControlBlock *Allocator::reallocBlock(MemoryControlBlock *mcb, std::size_t size) {
    if (mcb->fits(size)) {
        mcb->markBusy();
        this->split(mcb, size);
        this->freeBlocks.pop(mcb);
        return mcb;
    }

    // check if we can absorb next block and there will be enough space
    if (auto *next = MemoryControlBlock::next(mcb);
        !next->busy() && size <= mcb->size() + sizeof(MemoryControlBlock) + next->size()) {
        this->absorbNext(mcb);
        this->split(mcb, size); // mcb can be larger than we need after the absorption
        return mcb;
    }

    auto *oldMCB = mcb;
    mcb = this->allocBlock(size, 1);
    std::copy(reinterpret_cast<std::uint16_t *>(MemoryControlBlock::data(oldMCB)),
        reinterpret_cast<std::uint16_t *>(MemoryControlBlock::data(oldMCB) + oldMCB->size()),
        reinterpret_cast<std::uint16_t *>(MemoryControlBlock::data(mcb)));
    this->freeBlock(oldMCB);
    return mcb;
}

MemoryControlBlock *Allocator::allocChunk(std::size_t size) {
    std::size_t totalSize = math::roundUp(
        sizeof(MemoryControlBlock) + size + sizeof(MemoryControlBlock),
        system::PAGE_SIZE());

    auto *mcb = reinterpret_cast<MemoryControlBlock *>(system::mmap(totalSize));
    mcb->setSize(totalSize - sizeof(MemoryControlBlock) - sizeof(MemoryControlBlock));
    this->freeBlocks.prepend(mcb);

    auto *end = MemoryControlBlock::next(mcb);
    end->setPrev(mcb);
    end->markBusy();
    end->setSize(0);

    return mcb;
}

MemoryControlBlock* Allocator::shiftForward(MemoryControlBlock *mcb, std::size_t shift) noexcept {
    if (shift == 0) {
        return mcb;
    }

    if (shift >= sizeof(MemoryControlBlock) + 2) {
        auto *right = this->split(mcb, shift - sizeof(MemoryControlBlock));

        if (auto *prev = mcb->prev(); prev != nullptr && !prev->busy()) {
            this->absorbNext(prev);
        }

        return right;
    }

    auto storedMCB = *mcb;
    auto *shiftedMCB = reinterpret_cast<MemoryControlBlock*>(
           reinterpret_cast<std::uint8_t*>(mcb) + shift);

    if (this->freeBlocks.first() == mcb) {
        this->freeBlocks.setFirst(shiftedMCB);
    }

    MemoryControlBlock::next(mcb)->setPrev(shiftedMCB);
    auto *prev = mcb->prev();
    if (prev != nullptr) {
        prev->grow(shift);
    }
    shiftedMCB->setPrev(prev);
    MemoryControlBlock::setPrevFree(shiftedMCB, storedMCB.prevFree());
    MemoryControlBlock::setNextFree(shiftedMCB, storedMCB.nextFree());
    shiftedMCB->setSize(storedMCB.size() - shift);
    shiftedMCB->setBusy(storedMCB.busy());

    return shiftedMCB;
}

MemoryControlBlock* Allocator::split(MemoryControlBlock *mcb, std::size_t size) noexcept {
    if (size == 0) {
        return mcb;
    }

    if (!mcb->fits(size + sizeof(MemoryControlBlock) + 2)) {
        return mcb;
    }

    auto oldSize = mcb->size();
    mcb->setSize(size);

    auto *right = MemoryControlBlock::next(mcb);
    right->setSize(oldSize - size - sizeof(MemoryControlBlock));
    right->setPrev(mcb);

    auto *next = MemoryControlBlock::next(right);
    next->setPrev(right);
    if (!next->busy()) {
        this->absorbNext(right);
    }

    this->freeBlocks.prepend(right);
    return right;
}

void Allocator::absorbNext(MemoryControlBlock *mcb) noexcept {
    auto *next = MemoryControlBlock::next(mcb);
    this->freeBlocks.pop(next);
    MemoryControlBlock::next(next)->setPrev(mcb);
    mcb->grow(sizeof(MemoryControlBlock) + next->size());
}

void Allocator::free(std::uintptr_t ptr) {
    this->freeBlock(MemoryControlBlock::fromDataPtr(ptr));
}

void Allocator::freeBlock(MemoryControlBlock *mcb) {
    mcb->markFree();
    this->freeBlocks.prepend(mcb);

    if (MemoryControlBlock *next = MemoryControlBlock::next(mcb); !next->busy()) {
        this->absorbNext(mcb);
    }

    if (MemoryControlBlock *prev = mcb->prev(); prev != nullptr && !prev->busy()) {
        mcb = prev;
        this->absorbNext(mcb);
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
    std::uintptr_t to = next->busy() && next->empty()
        // end of chunk can be not page-aligned
        ? math::roundUp(MemoryControlBlock::data(next), system::PAGE_SIZE())
        // we do not want to corrunt next blocks in this page
        : math::roundDown(reinterpret_cast<std::uintptr_t>(next), system::PAGE_SIZE());

    if (to < from + system::PAGE_SIZE()) {
        // there is no free pages to delete
        return;
    }

    if (std::ptrdiff_t diff = from - MemoryControlBlock::data(mcb); diff >= 0) {
        // mcb is not the first in chunk since moved foreward
        if (diff >= static_cast<std::ptrdiff_t>(sizeof(MemoryControlBlock) + 2)) {
            // there is enough space for non-empty block
            mcb->setSize(diff - sizeof(MemoryControlBlock));
            auto *end = MemoryControlBlock::next(mcb);
            end->setPrev(mcb);
            end->markBusy();
            end->setSize(0);
        } else {
            // there is no space in current page
            mcb->markBusy();
            mcb->setSize(0);
            this->freeBlocks.pop(mcb);
        }
    } else { // block will be removed with pages
        this->freeBlocks.pop(mcb);
    }

    if (std::ptrdiff_t diff = reinterpret_cast<std::uintptr_t>(next) - to; diff >= 0) {
        // next is not the last in chunk since moved backward
        if (diff >= static_cast<std::ptrdiff_t>(sizeof(MemoryControlBlock) + 2)) {
            // there is enough space for non-empty block
            auto *first = reinterpret_cast<MemoryControlBlock *>(to);
            first->markFree();
            first->setSize(diff - sizeof(MemoryControlBlock) - sizeof(MemoryControlBlock));
            first->setPrev(nullptr);
            next->setPrev(first);
            this->freeBlocks.prepend(first);
        } else {
            // there is no space before next
            next->setPrev(nullptr);
        }
    }

    system::munmap(from, to - from);
}

std::size_t Allocator::shiftToAlignData(const MemoryControlBlock *mcb, std::size_t alignment) noexcept {
    auto data = MemoryControlBlock::data(mcb);
    return math::roundUp(data, alignment) - data;
}

} // namespace hse::memory
