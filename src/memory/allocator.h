#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "memory_control_block.h"
#include "random/random.h"

#include <cstddef>
#include <cstdint>

namespace hse::memory {
	// Allocator is responsible for managing allocated memory pages and chunks of blocks
	class Allocator {
	private:
        //PRNG generator which is used in every call to random value
        static sc69069_t randomGenerator;

		// firstFree is a pointer to first block in chain of free blocks
		MemoryControlBlock *firstFree;

		// prependFree prepends given block to start of chain of free blocks
		void prependFree(MemoryControlBlock*) noexcept;

		// searchFit returns first block in chain of free blocks
		// which fits given size
		// It returns nullptr if there is no such block
		MemoryControlBlock* searchFit(std::size_t) const noexcept;

		// popFree extracts given block from chain of free blocks
		// and moves firstFree if needed
		void popFree(MemoryControlBlock*) noexcept;

		// allocBlock returns block from chain of free blocks
		// or allocates memory for new one if needed
		MemoryControlBlock* allocBlock(std::size_t);

		// allocChunk allocates memory pages for new block with given size
		MemoryControlBlock* allocChunk(std::size_t);

		// freeBlock releases block back to chain of blocks,
		// merges it with its neighbors and unmaps free memory pages
		// within given block
		void freeBlock(MemoryControlBlock*);


		// tryUnmap tries to unmap memory pages within given block
		void tryUnmap(MemoryControlBlock*);


	public:
		// alloc allocates memory for holding given number of bytes
		std::uintptr_t alloc(std::size_t);

		// free deallocates memory pointed by given pointer
		void free(std::uintptr_t);

        // realloc reallocates memory for given ptr and size
        // it may resize current mcb, do nothing, squeeze current mcb or
        // allocate new mcb and copy data there depending on the input
        std::uintptr_t realloc(std::uintptr_t, std::size_t);
	};
}

#endif // ALLOCATOR_H
