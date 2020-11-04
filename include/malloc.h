#ifndef MALLOC_HPP
#define MALLOC_HPP

#ifdef __cplusplus
#include <cstddef>

namespace std {
    extern "C" {
#endif // __cplusplus
    #include <stddef.h>

    void* malloc(size_t);
    void free(void*);

#ifdef __cplusplus
    } // extern "C"
} // namespace std

#endif // __cplusplus

#endif // MALLOC_HPP
