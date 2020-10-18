#include "malloc/malloc.h"

#include <ios>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <forward_list>
#include <utility>
#include <system_error>

#include <iostream>
#include <iomanip>

namespace hse {
	const long PAGE_SIZE = ::sysconf(_SC_PAGESIZE);

	std::size_t roundUp(std::size_t num, std::size_t multiplier) noexcept {
		if (multiplier == 0)
			return num;
		std::size_t remainder = num % multiplier;
		if (remainder == 0)
			return num;
		return num + multiplier - remainder;
	}

	void* mmap(std::size_t size) {
		void *ptr = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (ptr == reinterpret_cast<void*>(-1))
			throw std::system_error(errno, std::system_category(), "mmap");
		std::cout << "allocated " << size << " bytes at " << std::hex << std::showbase << ptr 
			<< std::resetiosflags(std::ios_base::hex | std::ios_base::showbase) << std::endl;
		return ptr;
	}

	void munmap(void *addr, std::size_t len) {
		if (::munmap(addr, len) == -1)
			throw std::system_error(errno, std::system_category(), "munmap");
	}

	struct MemoryControlBlock {
		bool busy;
		std::size_t size; // in bytes
		MemoryControlBlock *prev;

		MemoryControlBlock *prevFree;
		MemoryControlBlock *nextFree;

		bool empty() const noexcept { return this->size == 0; }

		std::size_t totalSize() const noexcept { return sizeof(MemoryControlBlock) + this->size; }

		void setPrevFree(MemoryControlBlock *prevFree) noexcept {
			if ((this->prevFree = prevFree))
				this->prevFree->nextFree = this;
		}

		void setNextFree(MemoryControlBlock *nextFree) noexcept {
			if ((this->nextFree = nextFree))
				nextFree->prevFree = this;
		}

		MemoryControlBlock* next() noexcept {
			return reinterpret_cast<MemoryControlBlock*>(reinterpret_cast<char*>(this + 1) + this->size);
		}

		bool isLastFromChunk() const noexcept {
			return this->size == 0;
		}

		void popFromFree() noexcept {
			if (this->prevFree)
				this->prevFree->setNextFree(this->nextFree);
			else if (this->nextFree)
				this->nextFree->setPrevFree(this->prevFree);
			this->prevFree = nullptr;
			this->nextFree = nullptr;
		}

		bool canCut(std::size_t size) const noexcept {
			if (this->busy)
				return false;
			return this->size - size >= sizeof(MemoryControlBlock) + 1;
		}

		void cut(std::size_t size) noexcept {
			if (this->busy || !this->canCut(size))
				return;
			
			std::size_t oldSize = this->size;
			this->size = size;

			MemoryControlBlock *next = this->next();
			*next = {
				.busy = false,
				.size = oldSize - size - sizeof(MemoryControlBlock),
				.prev = this,
			};
			next->next()->prev = next;
			
			next->setNextFree(this->nextFree);
			next->setPrevFree(this);
			// this->setNextFree(next);
		}

		// MemoryControlBlock* cut(std::size_t size) noexcept {
		// 	std::size_t oldSize = this->size;
        //
		// 	MemoryControlBlock *oldNext = this->next();
		// 	this->size = size;
		// 	MemoryControlBlock *next = this->next();
		// 	*next = {
		// 		.busy = false,
		// 		.size = oldSize - size - sizeof(MemoryControlBlock),
		// 		.prev = this,
		// 	}; // TODO: initialization?
		// 	if (oldNext) // TODO: check boundaries?
		// 		oldNext->prev = next;
		// 	next->setNextFree(this->nextFree);
		// 	this->setNextFree(next);
		// 	return next;
		// }

		// void tryCut(std::size_t size) {
		// 	if (this->canCut(size))
		// 		this->cut(size);
		// 	// TODO: make in one method
		// }

		// void makeBusy()


		// MemoryControlBlock* pop() {
		//     if (MemoryControlBlock* next = this->next(); next)
		//        next->prev=this->prev;
		//     if (MemoryControlBlock *prevFree = this->prevFree; prevFree)
		//         prevFree->nextFree = this->nextFree;
		//     if (MemoryControlBlock *nextFree = this->nextFree; nextFree)
		//        nextFree->prevFree = this->prevFree;
		//     return this->prev;
		// }
		
		// MemoryControlBlock* prevFree() const { return this->prevFree_; }
		// MemoryControlBlock* nextFree() const { return this->nextFree_; }
		// MemoryControlBlock* setPrevFree(MemoryControlBlock* prev) { 
		//     MemoryControlBlock *old = this->prevFree_;
		//     this->prevFree_ = prev;
		//     return old;
		// }
		// MemoryControlBlock* setNextFree(MemoryControlBlock* next) { 
		//     MemoryControlBlock *old = this->nextFree_;
		//     this->nextFree_ = next;
		//     return old;
		// } 

		// void exclude() {
		//     if (MemoryControlBlock *prev = this->prevFree())
		//         prev->setNextFree(this->nextFree());
		//     if (MemoryControlBlock *next = this->nextFree())
		//         next->setPrevFree(this->prevFree());
		//     this->setPrevFree(nullptr); // TODO: return prev?
		//     this->setNextFree(nullptr); // TODO: return next?
		// }

	// public:
	//     bool busy() const { return this->busy_; }
	//     std::size_t len() const { return this->len_; }
	//     std::size_t cap() const { return this->cap_; }

		// returns pointer to next free block
		// MemoryControlBlock *occupy(std::size_t size) {
		//     this->busy_ = true;
		//     this->len_ = size;
		//     this->exclude();
		//     return this->nextFree_; // TODO: return pair?
		// }
		// // returns pointer to next free block
		// MemoryControlBlock* free() {
		//     this->busy_ = false;
		//     this->exclude();
		// }
	};

	class Allocator {
	private:
		MemoryControlBlock* firstFreeBlock;

		void setFirstFree(MemoryControlBlock *first) {
			first->setNextFree(this->firstFreeBlock);
			this->firstFreeBlock = first;
		}

	public:
		Allocator() {
			std::cout << "init allocator" << std::endl
				<< "sizeof(MCB) = " << sizeof(MemoryControlBlock) << " bytes" << std::endl
				<< "page size: " << PAGE_SIZE << std::endl;
		}

		void *alloc(std::size_t size) {
		  return reinterpret_cast<void *>(this->allocMCB(size) + 1);
		}

		MemoryControlBlock *searchFit(std::size_t size) {
			MemoryControlBlock *mcb;
			for (mcb = this->firstFreeBlock; mcb && mcb->size < size; mcb = mcb->nextFree);
			return mcb;
		}

		MemoryControlBlock *allocNewMCB(std::size_t size) {
			std::size_t spaceNeeded = sizeof(MemoryControlBlock) + size + sizeof(MemoryControlBlock);
				// for current mcb, its size
				// and for last mcb with
				// zero size at least
			std::size_t totalSize = roundUp(spaceNeeded, PAGE_SIZE);
			MemoryControlBlock *mcb = reinterpret_cast<MemoryControlBlock *>(mmap(totalSize));
			*mcb = {
				.size = totalSize - sizeof(MemoryControlBlock) - sizeof(MemoryControlBlock), // for current mcb and the last one
				.prev = nullptr,
			}; // TODO: initialization?
			mcb->setNextFree(this->firstFreeBlock);
			this->firstFreeBlock = mcb;
			// std::cout << "new mcb block in fresh chunk: " << std::hex
			// 	<< std::showbase << mcb
			// 	<< std::resetiosflags(std::ios_base::hex | std::ios_base::showbase)
			// 	<< " with size: " << mcb->size << std::endl;

			MemoryControlBlock *last = mcb->next();
			*last = {
				.busy = true, // means last in chunk 
				.size = 0, // means last in chunk
				.prev = mcb,
			};
			// std::cout << "last block in fresh chunk: " << std::hex
			// 	<< std::showbase << last
			// 	<< std::resetiosflags(std::ios_base::hex | std::ios_base::showbase) << std::endl;
			return mcb;
		}

		void popFromFree(MemoryControlBlock *mcb) noexcept {
			if (this->firstFreeBlock == mcb)
				this->firstFreeBlock = mcb->nextFree;
			mcb->popFromFree();
		}

		MemoryControlBlock *allocMCB(std::size_t size) {
			MemoryControlBlock *mcb = this->searchFit(size);
			if (!mcb) { // there is no suitable block
				mcb = this->allocNewMCB(size);
			}

			mcb->cut(size);

			this->popFromFree(mcb);
			mcb->busy = true;
			return mcb;
		}

		void free(void *ptr) {
			release(reinterpret_cast<MemoryControlBlock*>(ptr) - 1);
		}

		void mergeWithNext(MemoryControlBlock *mcb) noexcept {
			MemoryControlBlock *next = mcb->next();
			this->popFromFree(next);
			mcb->size += next->totalSize();
			next->next()->prev = mcb;
		}

		void prependFree(MemoryControlBlock *mcb) noexcept {
			mcb->setNextFree(this->firstFreeBlock);
			this->firstFreeBlock = mcb;
		}

		void release(MemoryControlBlock *mcb) {
			// TODO: remove all free chunks

			mcb->busy = false; // TODO: set only if do not merge with prev
			this->prependFree(mcb);

			if (MemoryControlBlock* next = mcb->next(); next && !next->busy) { // TODO: next is always not nullptr
				this->mergeWithNext(mcb);
			}
			if (MemoryControlBlock* prev = mcb->prev; prev && !prev->busy) {
				mcb = prev;
				this->mergeWithNext(mcb);
			}
            //
			// // check if this 
			// if (MemoryControlBlock *next = mcb->next();
			// 		mcb->prev == nullptr && next->busy && next->size == 0) { // TODO: size == 0 is enough
			// 	munmap(mcb, sizeof(MemoryControlBlock) + mcb->size + sizeof(MemoryControlBlock))
			// }
		}

		~Allocator() {} // TODO: unmap all chunks // TODO: it should be done by freeing all pointers by user
	};
    
    static Allocator allocator;

    void* malloc(std::size_t size) {
        return allocator.alloc(size);
    }
    void free(void *ptr) {
        allocator.free(ptr);
    }
}

namespace mymalloc { 
    struct mem_control_block {
        int is_available;
        int size;
    };

    static int has_initialized = 0;
    static void *managed_memory_start;
    static void *last_valid_address;

    void malloc_init() {
        last_valid_address = ::sbrk(0);
        managed_memory_start = last_valid_address;
        has_initialized = 1;
    }

    void* mem_alloc(std::size_t size) {
        if (!has_initialized)
            malloc_init();

        size += sizeof(mem_control_block);
        
    
        void *current_location = managed_memory_start;
        void *memory_location = 0;

        while (current_location != last_valid_address) {
            auto current_mcb = reinterpret_cast<mem_control_block*>(current_location);
            if (current_mcb->is_available && current_mcb->size >= size) {
                current_mcb->is_available = 0;
                memory_location = current_location;
            }
            
            current_location = reinterpret_cast<char*>(current_location) + current_mcb->size;
        }

        if (!memory_location) {
            sbrk(size);
            memory_location = last_valid_address;
            last_valid_address = reinterpret_cast<char*>(last_valid_address) + size;
            auto current_mcb = reinterpret_cast<mem_control_block*>(memory_location);
            current_mcb->is_available = 0;
            current_mcb->size = size;
        }

        memory_location = reinterpret_cast<char*>(memory_location) + sizeof(mem_control_block);
        
        return memory_location;
    }

    void mem_free(void *ptr) {
        auto mcb = reinterpret_cast<mem_control_block*>(reinterpret_cast<char*>(ptr) - sizeof(mem_control_block));

        mcb->is_available = 1;
    }
}
