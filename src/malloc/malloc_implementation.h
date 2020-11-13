#ifndef MALLOC_HPP
#define MALLOC_HPP

#include <cstddef>

namespace hse {

int sum(std::size_t) noexcept;

void* malloc(std::size_t) noexcept;
void* calloc(std::size_t count, std::size_t size) noexcept;
void* realloc(void *ptr, std::size_t size) noexcept;
void* aligned_alloc(std::size_t alignment, std::size_t size) noexcept;
void free(void *) noexcept;


} // namespace hse


#endif // MALLOC_HPP
