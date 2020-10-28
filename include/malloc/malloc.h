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

void* hse_malloc(unsigned long);
void hse_free(void*);

#ifdef __cplusplus
}
#endif


#endif // MALLOC_HPP
