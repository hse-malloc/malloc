#include "allocator.h"
#include "memory_control_block.h"
#include "math/math.h"
#include "system/system.h"

#include <cstddef>
#include <cstdint>

namespace hse::memory {
	void Allocator::prependFree(MemoryControlBlock *mcb) noexcept {
		mcb->setNextFree(this->firstFree);
		this->firstFree = mcb;
	}

	MemoryControlBlock* Allocator::searchFit(std::size_t size) const noexcept {
		MemoryControlBlock *mcb;
		for (mcb = this->firstFree; mcb && !mcb->fits(size); mcb = mcb->nextFree());
		return mcb;
	}

	void Allocator::popFree(MemoryControlBlock *mcb) noexcept {
		if (this->firstFree == mcb)
			this->firstFree = mcb->nextFree();
		mcb->popFree();
	}

	std::uintptr_t Allocator::alloc(std::size_t size) {
		return reinterpret_cast<std::uintptr_t>(this->allocBlock(size) + 1);
	}

	MemoryControlBlock* Allocator::allocBlock(std::size_t size) {
		MemoryControlBlock *mcb = this->searchFit(size);
		if (!mcb)
			mcb = this->allocChunk(size);

		mcb->split(size);

		this->popFree(mcb);
		mcb->setBusy();
		return mcb;
	}

	MemoryControlBlock* Allocator::allocChunk(std::size_t size) {
		std::size_t needSpace = sizeof(MemoryControlBlock) + size + sizeof(MemoryControlBlock);
		std::size_t totalSize = math::roundUp(needSpace, system::PAGE_SIZE);
		
		MemoryControlBlock *mcb = reinterpret_cast<MemoryControlBlock*>(system::mmap(totalSize));
		mcb->setSize(totalSize - sizeof(MemoryControlBlock) - sizeof(MemoryControlBlock));
		this->prependFree(mcb);
		MemoryControlBlock* end = mcb->next();
		end->setPrev(mcb);
		end->makeEndOfChunk();
		return mcb;
	}

	void Allocator::free(std::uintptr_t ptr) {
		this->freeBlock(reinterpret_cast<MemoryControlBlock*>(ptr) - 1);
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
			from = math::roundDown(reinterpret_cast<std::uintptr_t>(mcb), system::PAGE_SIZE);
		else
			// we do not want to corrupt previous blocks in this page
			from = math::roundUp(reinterpret_cast<std::uintptr_t>(mcb + 1), system::PAGE_SIZE);

		MemoryControlBlock *next = mcb->next();
		std::uintptr_t to;
		if (next->endOfChunk())
			// end of chunk can be not page-aligned
			to = math::roundUp(reinterpret_cast<std::uintptr_t>(next + 1), system::PAGE_SIZE);
		else
			// we do not want to corrunt next blocks in this page
			to = math::roundDown(reinterpret_cast<std::uintptr_t>(next), system::PAGE_SIZE);
		
		std::ptrdiff_t len = to - from;
		if (len < system::PAGE_SIZE)
			// there is no free pages to delete
			return;
		
		// store block in case it is in pages we are to delete
		MemoryControlBlock mcbStored = *mcb;

		system::munmap(reinterpret_cast<std::uintptr_t>(from), len);

		if (std::ptrdiff_t diff = from - reinterpret_cast<std::uintptr_t>(mcb + 1); diff >= 0) {
			// mcb was not first in chunk since moved foreward
			if (diff >= static_cast<std::ptrdiff_t>(1 + sizeof(MemoryControlBlock))) {
				// there is enough space for non-empty block
				mcb->setSize(diff - sizeof(MemoryControlBlock));
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

		if (std::ptrdiff_t diff = reinterpret_cast<std::uintptr_t>(next) - to; diff >= 0) {
			// next was not last in chunk since moved backward
			if (diff >= static_cast<std::ptrdiff_t>(sizeof(MemoryControlBlock) + 1)) {
				// there is enough space for non-empty block
				MemoryControlBlock *first = reinterpret_cast<MemoryControlBlock*>(to);
				first->setFree();
				first->setSize(diff - sizeof(MemoryControlBlock) - sizeof(MemoryControlBlock));
				first->makeFirstInChunk();
				next->setPrev(first);
				this->prependFree(first);
			} else
				// there is no space before next
				next->makeFirstInChunk();
		}
	}
}
