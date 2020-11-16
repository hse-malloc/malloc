#include "system.h"

#ifdef HAVE_MMAP
#include <sys/mman.h>
#elif defined HAVE_VIRTUAL_ALLOC
#include <memoryapi.h>
#include <sysinfoapi.h>
#define _SC_PAGE_SIZE 0
#endif

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <sys/types.h>
#include <system_error>
#include <unistd.h>

namespace hse::system {

// sysconf returns the value configurable system variable by given name
std::uint64_t sysconf(int name) {
#ifdef HAVE_MMAP
    std::uint64_t val = ::sysconf(name);
#elif defined HAVE_VIRTUAL_ALLOC
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    std::uint64_t val = sysInfo.dwPageSize;
#endif
    if (val == -1UL) {
        throw std::system_error(errno, std::system_category(),
#ifdef HAVE_MMAP
            "sysconf"
#elif defined HAVE_VERTUAL_ALLOC
            "GetSystemInfo"
#endif
        );
    }
    return val;
}

std::uintptr_t mmap(std::size_t size) {    
    void *ptr =
#ifdef HAVE_MMAP
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)
#elif defined HAVE_VIRTUAL_ALLOC
        ::VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)
#endif
    ;
    if (ptr ==
#ifdef HAVE_MMAP
            reinterpret_cast<void *>(-1)
#elif defined HAVE_VIRTUAL_ALLOC
            nullptr
#endif
        ) {
        throw std::system_error(errno, std::system_category(),
#ifdef HAVE_MMAP
            "mmap"
#elif defined HAVE_VIRTUAL_ALLOC
            "VirtualAlloc"
#endif
        );
    }
    return reinterpret_cast<std::uintptr_t>(ptr);
}

void munmap(std::uintptr_t addr, std::size_t len) {
    if (
#ifdef HAVE_MMAP
    ::munmap(reinterpret_cast<void *>(addr), len) == -1
#elif defined HAVE_VIRTUAL_ALLOC
    ::VirtualFree(reinterpret_cast<void *>(addr), 0, MEM_RELEASE) == 0
#endif
    ) {
        throw std::system_error(errno, std::system_category(),
#ifdef HAVE_MMAP
            "munmap"
#elif defined HAVE_VIRTUAL_ALLOC
            "VirtualFree"
#endif
        );
    }
}

extern std::size_t PAGE_SIZE() {
    static std::size_t PAGE_SIZE = system::sysconf(_SC_PAGE_SIZE);
    return PAGE_SIZE;
}

} // namespace hse::system
