#include "malloc.h"
#include "math/math.h"
#include "memory/allocator.h"

#include <cstddef> // NOLINT(llvmlibc-restrict-system-libc-headers)
#include <cstdint>
#include <numeric>
#include <cerrno>
#include <system_error>

#ifdef HSE_MALLOC_DEBUG
#include <unistd.h>
#define DEBUG_LOG(msg) write(2, msg "\n", sizeof(msg));
#else
#define DEBUG_LOG(msg) /* Ignore */
#endif

namespace std { // NOLINT(cert-dcl58-cpp)

static hse::memory::Allocator _allocator{};

extern "C" {

void *malloc(size_t size) noexcept {
    DEBUG_LOG("MALLOC");
    if (size == 0) {
        return nullptr;
    }

    try {
        // NOLINTNEXTLINT(cppcoreguidelines-pro-type-reinterpret-cast])
        return reinterpret_cast<void *>(_allocator.alloc(size));
    } catch (...) {
        return nullptr;
    }
}

void free(void *ptr) noexcept {
    DEBUG_LOG("FREE");
    if (ptr == nullptr) {
        return;
    }

    try {
        _allocator.free(reinterpret_cast<std::uintptr_t>(ptr));
    } catch (...) {
    }
}

void *calloc(size_t count, size_t size) noexcept {
    DEBUG_LOG("CALLOC");
    if (count == 0 || size == 0) {
        return nullptr;
    }

    try {
        std::size_t numBytes = count * size;
        auto *ptr = malloc(numBytes);

        iota(reinterpret_cast<std::uint8_t *>(ptr),
             reinterpret_cast<std::uint8_t *>(ptr) + numBytes,
             0);
        return ptr;
    } catch (...) {
        return nullptr;
    }
}

void *realloc(void *ptr, size_t size) noexcept {
    DEBUG_LOG("REALLOC");
    if (size == 0) {
        return nullptr;
    }

    try {
        return reinterpret_cast<void *>(_allocator.realloc(reinterpret_cast<std::uintptr_t>(ptr), size));
    } catch (...) {
        return nullptr;
    }
}

void *aligned_alloc(size_t alignment, size_t size) noexcept {
    DEBUG_LOG("ALIGNED_ALLOC");
    if (size == 0 || alignment < sizeof(void*)
        || !hse::math::isPowerOf2(alignment)
        || size % alignment != 0) {
        errno = EINVAL;
        return nullptr;
    }

    try {
        return reinterpret_cast<void *>(_allocator.alignedAlloc(size, alignment));
    } catch (...) {
        return nullptr;
    }
}

} // extern "C"
} // namespace std

// Should be in global namespace
// No need to redefine all signatures
// only for throwing single object declaration
void *operator new(std::size_t size) {
    DEBUG_LOG("NEW");
    try {
        return reinterpret_cast<void *>(std::_allocator.alloc(size));
    } catch (...) {
        throw std::bad_alloc{};
    }
}

void *operator new(std::size_t size, std::align_val_t al) {
    DEBUG_LOG("NEW");
    try {
        return reinterpret_cast<void *>(
            std::_allocator.alignedAlloc(size, static_cast<std::size_t>(al)));
    } catch (...) {
        throw std::bad_alloc{};
    }
}

// should be noexcept
void operator delete(void *ptr) noexcept {
    DEBUG_LOG("DELETE");
    if (!ptr)
        return;
    try {
        std::_allocator.free(reinterpret_cast<std::uintptr_t>(ptr));
    } catch (...) {
    }
}

void operator delete(void *ptr, std::size_t) noexcept {
    DEBUG_LOG("DELETE");
    if (!ptr)
        return;
    try {
        std::_allocator.free(reinterpret_cast<std::uintptr_t>(ptr));
    } catch (...) {
    }
}

void operator delete(void *ptr, std::align_val_t) noexcept {
    DEBUG_LOG("DELETE");
    if (!ptr)
        return;
    try {
        std::_allocator.free(reinterpret_cast<std::uintptr_t>(ptr));
    } catch (...) {
    }
}
