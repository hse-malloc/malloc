#ifndef MALLOC_HPP
#define MALLOC_HPP

#include <cstddef>

namespace mymalloc {
	void* mem_alloc(std::size_t);
	void mem_free(void*);
}

namespace hse {
	void* malloc(std::size_t);
	void free(void*);
}

#endif // MALLOC_HPP
