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

extern "C" void* hse_malloc(unsigned long size) noexcept {
    try {
        return hse::malloc(size);
    }
    catch (...) {
        return nullptr;
    }
}

extern "C" void free(void* ptr) noexcept {
    try {
        hse::free(ptr);
    }
    catch (...) {
        return ;
    }
}

