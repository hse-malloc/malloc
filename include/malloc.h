#ifndef MALLOC_HPP
#define MALLOC_HPP


// defining C and C++ function specificators
#ifdef __cplusplus
#define __NOEXCEPT__ noexcept
#else
#define __NOEXCEPT__ /* Ignore */
#endif


#ifdef __cplusplus
#include <cstddef>

namespace std {
    extern "C" {
#endif // __cplusplus
    #include <stddef.h>

    void* malloc(size_t) __NOEXCEPT__;
    void free(void*) __NOEXCEPT__;
    void* calloc(size_t num, size_t size ) __NOEXCEPT__;
    void* realloc(void* ptr, size_t size ) __NOEXCEPT__;

#ifdef __cplusplus
    } // extern "C"
} // namespace std

#endif // __cplusplus

#endif // MALLOC_HPP
