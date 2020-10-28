#ifndef MALLOC_HPP
#define MALLOC_HPP

#include <cstddef>

namespace hse {
	void* malloc(std::size_t);
	void free(void*);
}

#endif // MALLOC_HPP
