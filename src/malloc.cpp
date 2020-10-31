#include "malloc.h"
#include "memory/allocator.h"

#include <cstddef>
#include <cstdint>

namespace hse {
	memory::Allocator allocator;

	void* malloc(size_t size) {
		return reinterpret_cast<void*>(allocator.alloc(size));
	}

	void free(void *ptr) {
		allocator.free(reinterpret_cast<std::uintptr_t>(ptr));
	}
} // namespace hse

extern "C" {
	#include <stddef.h>

	void* malloc(size_t size) try {
		return hse::malloc(size);
	} catch (...) {
		return nullptr;
	}

	void free(void *ptr) try {
		hse::free(ptr);
	} catch (...) {
		return;
	}
} // extern "C"

