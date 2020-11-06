#include "malloc.h"
#include "memory/allocator.h"

#include <cstddef>
#include <cstdint>

namespace std {
	hse::memory::Allocator _allocator{};

    extern "C" {
        void* malloc(size_t size) noexcept {
            try {
                return reinterpret_cast<void*>(_allocator.alloc(size));
            } catch (...) {
				return nullptr;
			}
        }

        void free(void *ptr) noexcept {
            try {
                _allocator.free(reinterpret_cast<std::uintptr_t>(ptr));
            } catch (...) {}
        }
    }
} // namespace std

