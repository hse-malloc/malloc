#ifndef MALLOC_HPP
#define MALLOC_HPP

#include <cstddef>

namespace hse {

[[nodiscard]] void* malloc(std::size_t) noexcept;
[[nodiscard]] void* calloc(std::size_t count, std::size_t size) noexcept;
[[nodiscard]] void* realloc(void *ptr, std::size_t size) noexcept;
[[nodiscard]] void* aligned_alloc(std::size_t alignment, std::size_t size) noexcept;
void free(void *) noexcept;
[[nodiscard]] std::size_t malloc_usable_size (void *ptr) noexcept;

} // namespace hse


#endif // MALLOC_HPP
