#include "malloc.h"
#include "memory/allocator.h"

#include <cstddef>
#include <cstdint>
#include <system_error>

#ifdef HSE_MALLOC_DEBUG
#include <unistd.h>
#define DEBUG_LOG(str) write(2, str"\n", sizeof(str));
#else
#define DEBUG_LOG(str) /* Ignore */
#endif

namespace std {
    static hse::memory::Allocator _allocator{};

    extern "C" {
        void* malloc(size_t size) noexcept {
            DEBUG_LOG("MALLOC");
            if(!size) return nullptr;
            try {
                return reinterpret_cast<void*>(_allocator.alloc(size));
            } catch (...) {
				return nullptr;
			}
        }

        void free(void *ptr) noexcept {
            DEBUG_LOG("FREE");
            if(!ptr) return;
            try {
                _allocator.free(reinterpret_cast<std::uintptr_t>(ptr));
            } catch (...) {}
        }

        void* calloc(size_t num, size_t size ) noexcept
        {
            DEBUG_LOG("CALLOC");
            if(!num || !size) return nullptr;
            try {
                // getting the pointer
                std::size_t count_bytes = num*size;
                auto ptr = _allocator.alloc(count_bytes);
                // initializing with 0
                for(std::size_t i=0; i<count_bytes; ++i)
                    *(reinterpret_cast<std::uint8_t*>(ptr + i)) = 0;
                return reinterpret_cast<void*>(ptr);

            } catch (...) { return nullptr; }

        }
    }
} // namespace std


// Should be in global namespace
// No need to redefine all signatures
// only for throwing single object declaration
void* operator new(std::size_t size)
{
    DEBUG_LOG("NEW");
    try {
        return reinterpret_cast<void*>(std::_allocator.alloc(size));
    } catch (...) {
        throw std::bad_alloc{};
    }
}


// should be noexcept
void operator delete(void* ptr) noexcept
{
    DEBUG_LOG("DELETE");
    if (!ptr) return;
    try {
        std::_allocator.free(reinterpret_cast<std::uintptr_t>(ptr));
    } catch (...) { }
}
