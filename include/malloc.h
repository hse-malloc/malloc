#ifndef MALLOC_HPP
#define MALLOC_HPP

#ifdef __cplusplus
#include <cstddef>

namespace hse {
    extern "C" {
#endif // __cplusplus

    void* malloc(std::size_t);
    void free(void*);

#ifdef __cplusplus
    } // extern "C"
} // namespace hse
#endif // __cplusplus

#endif // MALLOC_HPP
