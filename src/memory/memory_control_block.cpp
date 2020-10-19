#include "memory_control_block.h"

#include <cstddef>
#include <cstdint>

namespace hse::memory {
	MemoryControlBlock::MemoryControlBlock(bool busy, 
		std::size_t size,
		MemoryControlBlock *prev,
		MemoryControlBlock *prevFree,
		MemoryControlBlock *nextFree): 
			busy_(busy),
			size_(size),
			prev_(prev) {
		this->setPrevFree(prevFree);
		this->setNextFree(nextFree);
	}

	std::size_t MemoryControlBlock::size() const noexcept {
		return this->size_;
	}
	
	bool MemoryControlBlock::empty() const noexcept {
		return this->size() == 0;
	}
	
	void MemoryControlBlock::setSize(std::size_t size) noexcept {
		this->size_ = size;
	}
	
	void MemoryControlBlock::grow(std::size_t size) noexcept {
		this->size_ += size;
	}
	
	bool MemoryControlBlock::fits(std::size_t size) const noexcept {
		return this->size() >= size;
	}
	
	MemoryControlBlock* MemoryControlBlock::split(std::size_t size) noexcept {
		if (this->busy() || !this->fits(size + sizeof(MemoryControlBlock) + 1))
			return nullptr;

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
		return this->busy_;
	}
	
	void MemoryControlBlock::setBusy() noexcept { 
		this->busy_ = true;
		this->popFree();
	}
	
	void MemoryControlBlock::setFree() noexcept {
		this->busy_ = false;
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
