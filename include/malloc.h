#ifndef MALLOC_HPP
#define MALLOC_HPP

#ifdef __cplusplus
#include <cstddef>
#include <new>

namespace hse {
#else
#include <stddef.h>
#endif // __cplusplus
    void* malloc(size_t);
    void free(void*);
#ifdef __cplusplus
} // namespace hse
#endif // __cplusplus

#endif // MALLOC_HPP
