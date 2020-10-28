#include "memory_control_block.h"
#include "math/math.h"

#include <cstddef>
#include <cstdint>

namespace hse::memory {
	MemoryControlBlock::MemoryControlBlock(
		std::size_t size,
		MemoryControlBlock *prev,
		MemoryControlBlock *prevFree,
		MemoryControlBlock *nextFree): 
			prev_(prev) {
		this->setSize(size);
		this->setPrevFree(prevFree);
		this->setNextFree(nextFree);
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
	
	bool MemoryControlBlock::empty() const noexcept {
		return this->size() == 0;
	}
	
	std::size_t MemoryControlBlock::setSize(std::size_t size) noexcept {
		return (this->size_ = math::roundUp<std::size_t>(size, 2) | this->busy());
	}
	
	std::size_t MemoryControlBlock::grow(std::size_t size) noexcept {
		return this->setSize(this->size() + size);
	}
	
	bool MemoryControlBlock::fits(std::size_t size) const noexcept {
		return this->size() >= size;
	}
	
	MemoryControlBlock* MemoryControlBlock::split(std::size_t size) noexcept {
		size = math::roundUp<std::size_t>(size, 2);
		if (this->busy() || !this->fits(size + MemoryControlBlock::spaceNeeded(1)))
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
	
	MemoryControlBlock* MemoryControlBlock::prev() const noexcept {
		return this->prev_;
	}

	void MemoryControlBlock::setPrev(MemoryControlBlock* prev) noexcept {
		this->prev_ = prev;
	}

	MemoryControlBlock* MemoryControlBlock::next() const noexcept {
		return reinterpret_cast<MemoryControlBlock*>(reinterpret_cast<std::uintptr_t>(this + 1) + this->size());
	}

	void MemoryControlBlock::absorbNext() noexcept {
		MemoryControlBlock *next = this->next();
		next->popFree();
		next->next()->setPrev(this);
		this->grow(sizeof(MemoryControlBlock) + next->size());
	}

	void MemoryControlBlock::setPrevFree(MemoryControlBlock *prevFree) noexcept {
		if ((this->prevFree_ = prevFree))
			prevFree->nextFree_ = this;
	}

	MemoryControlBlock* MemoryControlBlock::nextFree() const noexcept {
		return this->nextFree_;
	}

	void MemoryControlBlock::setNextFree(MemoryControlBlock *nextFree) noexcept {
		if ((this->nextFree_ = nextFree))
			nextFree->prevFree_ = this;
	}

	void MemoryControlBlock::popFree() noexcept {
		if (this->prevFree_)
			this->prevFree_->setNextFree(this->nextFree());
		else if (MemoryControlBlock *nextFree = this->nextFree(); nextFree)
			nextFree->setPrevFree(this->prevFree_);
		this->setPrevFree(nullptr);
		this->setNextFree(nullptr);
	}

	bool MemoryControlBlock::firstInChunk() const noexcept {
		return !this->prev();
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
}
