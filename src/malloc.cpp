#include "malloc.h"
#include "memory/allocator.h"

#include <cstddef>
#include <cstdint>

namespace hse {
	memory::Allocator allocator;

	void* malloc(std::size_t size) {
		return reinterpret_cast<void*>(allocator.alloc(size));
	}

	void free(void *ptr) {
		allocator.free(reinterpret_cast<std::uintptr_t>(ptr));
	}
}
