#ifndef SYSTEM_H
#define SYSTEM_H

#include <cstddef>
#include <cstdint>

namespace hse::system {
	// PAGE_SIZE is a page size of memory
	extern const std::size_t PAGE_SIZE;

	// mmap allocates integer number of PRIVATE ANONYMOUS pages in memory
	// for READ and WRITE permissions capable of holding size bytes
	std::uintptr_t mmap(std::size_t size);

	// munmap removes mappings for all pages containing the part of indicated range
	void munmap(std::uintptr_t addr, std::size_t len);
}

#endif // SYSTEM_H
