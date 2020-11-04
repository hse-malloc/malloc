#include "malloc.h"
#include "memory/allocator.h"

#include <cstddef>
#include <cstdint>

namespace hse {
    memory::Allocator allocator{};

    extern "C" {
        void* malloc(std::size_t size) {
            try {
                return reinterpret_cast<void*>(allocator.alloc(size));
            } catch (...) {
				return nullptr;
			}
        }

        void free(void *ptr) {
            try {
                allocator.free(reinterpret_cast<std::uintptr_t>(ptr));
            } catch (...) {}
        }
    }
} // namespace hse

