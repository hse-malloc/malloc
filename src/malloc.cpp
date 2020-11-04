#include "malloc.h"
#include "memory/allocator.h"

#include <cstddef>
#include <cstdint>

namespace hse {
    memory::Allocator allocator{};
    throw_tag_t throw_tag{};

    void* malloc(std::size_t size, const throw_tag_t& ) {
		return reinterpret_cast<void*>(allocator.alloc(size));
	}

    void free(void *ptr, const throw_tag_t&) {
		allocator.free(reinterpret_cast<std::uintptr_t>(ptr));
	}

    extern "C"
    {
        void* malloc(std::size_t size) noexcept {
            try {
                return reinterpret_cast<void*>(allocator.alloc(size));
            } catch (...) { return nullptr; }
        }

        void free(void *ptr)  noexcept {
            try {
                allocator.free(reinterpret_cast<std::uintptr_t>(ptr));
            } catch (...) {}
        }

    }

} // namespace hse


