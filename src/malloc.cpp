#include "malloc.h"
#include "math/math.h"
#include "memory/allocator.h"

#include <cstddef> // NOLINT(llvmlibc-restrict-system-libc-headers)
#include <cstdint>
#include <numeric>
#include <cerrno>

#ifdef HSE_MALLOC_DEBUG
#include <unistd.h>
#define DEBUG_LOG(msg) ::write(2, msg "\n", sizeof(msg));
#else
#define DEBUG_LOG(msg) /* Ignore */
#endif

namespace hse {

static hse::memory::Allocator _allocator{};


void *malloc(std::size_t size) noexcept {
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

void *calloc(std::size_t count, std::size_t size) noexcept {
    DEBUG_LOG("CALLOC");
    if (count == 0 || size == 0) {
        return nullptr;
    }

    try {
        std::size_t numBytes = count * size;
        auto *ptr = hse::malloc(numBytes);

        std::iota(reinterpret_cast<std::uint8_t *>(ptr),
             reinterpret_cast<std::uint8_t *>(ptr) + numBytes,
             0);
        return ptr;
    } catch (...) {
        return nullptr;
    }
}

void *realloc(void *ptr, std::size_t size) noexcept {
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

void *aligned_alloc(std::size_t alignment, std::size_t size) noexcept {
    DEBUG_LOG("ALIGNED_ALLOC");
    if (size == 0 || alignment < sizeof(void*)
        || !hse::math::isPowerOf2(alignment)
        || size % alignment != 0) {
        errno = EINVAL;
        return nullptr;
    }

    try {
        return reinterpret_cast<void *>(_allocator.alloc(size, alignment));
    } catch (...) {
        return nullptr;
    }
}

} // namespace hse
