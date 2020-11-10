#include "system.h"


#ifdef HAVE_MMAP
#include <sys/mman.h>
#elif defined HAVE_VIRTUAL_ALLOC
#include <memoryapi.h>
#include <sysinfoapi.h>
#define _SC_PAGE_SIZE 0
#endif

#include <sys/types.h>
#include <unistd.h>

#include <cstddef>
#include <cstdint>
#include <cerrno>
#include <system_error>

namespace hse::system {

// sysconf returns the value configurable system variable by given name
long sysconf(int name) {
    long val;
#ifdef HAVE_MMAP
    val = ::sysconf(name);
    if (val == -1)
        throw std::system_error(errno, std::system_category(), "sysconf");
#elif defined HAVE_VIRTUAL_ALLOC
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    val = sysInfo.dwPageSize;
    if (val == -1)
        throw std::system_error(errno, std::system_category(), "GetSystemInfo");
#endif
    return val;
}

std::uintptr_t mmap(std::size_t size) {    
    void *ptr;
#ifdef HAVE_MMAP
    ptr = ::mmap(nullptr, size, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == reinterpret_cast<void *>(-1))
        throw std::system_error(errno, std::system_category(), "mmap");
#elif defined HAVE_VIRTUAL_ALLOC
    ptr = ::VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (ptr == nullptr)
        throw std::system_error(errno, std::system_category(), "VirtualAlloc");
#endif
    return reinterpret_cast<std::uintptr_t>(ptr);
}

void munmap(std::uintptr_t addr, std::size_t len) {
#ifdef HAVE_MMAP
    if (::munmap(reinterpret_cast<void *>(addr), len) == -1)
        throw std::system_error(errno, std::system_category(), "munmap");
#elif defined HAVE_VIRTUAL_ALLOC
    if (::VirtualFree(reinterpret_cast<void *>(addr), 0, MEM_RELEASE) == 0)
        throw std::system_error(errno, std::system_category(), "VirtualFree");
#endif
}

extern std::size_t PAGE_SIZE() {
    static std::size_t PAGE_SIZE = system::sysconf(_SC_PAGE_SIZE);
    return PAGE_SIZE;
}

} // namespace hse::system
