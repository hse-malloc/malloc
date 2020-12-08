#include "system/system.h"
#include "math/math.h"
#include "memory/allocator.h"

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <malloc.h>
#include <new>
#include <numeric>

namespace std { // NOLINT(cert-dcl58-cpp)

static hse::memory::Allocator a{};

extern "C" {

void *malloc(size_t size) noexcept { return hse::malloc(size); }

void free(void *ptr) noexcept { return hse::free(ptr); }

void *calloc(size_t count, size_t size) noexcept { return hse::calloc(count, size); }

void *realloc(void *ptr, size_t size) noexcept { return hse::realloc(ptr, size); }

void *aligned_alloc(size_t alignment, size_t size) noexcept { return hse::aligned_alloc(alignment, size); }

void *memalign(size_t alignment, size_t size) noexcept { return aligned_alloc(alignment, size); }

void *pvalloc(size_t size) noexcept {
    std::size_t page_size = hse::system::PAGE_SIZE();
    return hse::aligned_alloc(page_size, hse::math::roundUp(size, page_size));
}

void *valloc(size_t size) noexcept { return pvalloc(size); }

size_t malloc_usable_size (void *ptr) noexcept { return hse::malloc_usable_size(ptr);}

int posix_memalign(void **memptr, size_t alignment, size_t size){
    void *result = aligned_alloc(alignment, size);
    if(result==nullptr) // errors should be passed
        return -1;
    *memptr = result;
    return 0;
}

} // extern "C"
} // namespace std

// Should be in global namespace
// No need to redefine all signatures
// only for throwing single object declaration
void *operator new(std::size_t size) {
    void *ptr = std::malloc(size);
    if (ptr == nullptr) {
        throw std::bad_alloc{};
    }
}

void *operator new(std::size_t size, std::align_val_t al) {
    void *ptr = hse::aligned_alloc(static_cast<std::size_t>(al), size);;
    if (ptr == nullptr) {
        throw std::bad_alloc{};
    }
}

// should be noexcept
void operator delete(void *ptr) noexcept { std::free(ptr); }

void operator delete(void *ptr, std::size_t) noexcept { std::free(ptr); }

void operator delete(void *ptr, std::align_val_t) noexcept { std::free(ptr); }
