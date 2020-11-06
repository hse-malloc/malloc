#include "system.h"

#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstddef>
#include <cstdint>
#include <system_error>

namespace hse::system {

// sysconf returns the value configurable system variable by given name
long sysconf(int name) {
    long val = ::sysconf(name);
    if (val == -1)
        throw std::system_error(errno, std::system_category(), "sysconf");
    return val;
}

std::uintptr_t mmap(std::size_t size) {
    void *ptr = ::mmap(nullptr, size, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == reinterpret_cast<void *>(-1))
        throw std::system_error(errno, std::system_category(), "mmap");
    return reinterpret_cast<std::uintptr_t>(ptr);
}

void munmap(std::uintptr_t addr, std::size_t len) {
    if (::munmap(reinterpret_cast<void *>(addr), len) == -1)
        throw std::system_error(errno, std::system_category(), "munmap");
}

extern std::size_t PAGE_SIZE() {
    static std::size_t PAGE_SIZE = hse::system::sysconf(_SC_PAGE_SIZE);
    return PAGE_SIZE;
}
} // namespace hse::system
