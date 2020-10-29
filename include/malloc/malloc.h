#ifndef MALLOC_HPP
#define MALLOC_HPP

#ifdef __cplusplus

#include <cstddef>

namespace hse {
    void* malloc(std::size_t);
    void free(void*);
}

extern "C" {
#endif

void* malloc(unsigned long)
#if __cplusplus
noexcept
#endif
;
void free(void*)
#if __cplusplus
noexcept
#endif
;

#ifdef __cplusplus
}
#endif


#endif // MALLOC_HPP
