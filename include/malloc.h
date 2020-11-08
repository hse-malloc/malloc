#ifndef MALLOC_HPP
#define MALLOC_HPP

#ifdef __cplusplus
#if (__cplusplus >= 201103L) // C++11
#define __NOEXCEPT__ noexcept
#else
#define __NOEXCEPT__ throw()
#endif
#else
#define __NOEXCEPT__ /* Ignore */
#endif

#ifdef __cplusplus
#include <cstddef> // NOLINT(llvmlibc-restrict-system-libc-headers)

namespace std { // NOLINT(cert-dcl58-cpp)
extern "C" {
#endif // __cplusplus
#include <stddef.h> // NOLINT(llvmlibc-restrict-system-libc-headers)

void *malloc(size_t) __NOEXCEPT__;
void free(void *) __NOEXCEPT__;
void *calloc(size_t count, size_t size) __NOEXCEPT__;
void *realloc(void *ptr, size_t size) __NOEXCEPT__;
void *aligned_alloc(size_t alignment, size_t size) __NOEXCEPT__;

#ifdef __cplusplus
} // extern "C"
} // namespace std

#endif // __cplusplus

#endif // MALLOC_HPP
