#ifndef MALLOC_HPP
#define MALLOC_HPP

#ifdef __cplusplus
#include <cstddef>

namespace hse {

struct throw_tag_t{};

extern throw_tag_t throw_tag;


    extern "C" {

#endif // __cplusplus

extern
void*
malloc(unsigned long)

#ifdef __cplusplus
noexcept
#endif
;

extern
void
free(void*)
#ifdef __cplusplus
noexcept
#endif
;

#ifdef __cplusplus
    } // extern "C"

} // namespace hse
#endif // __cplusplus

#endif // MALLOC_HPP
