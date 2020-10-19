#include <malloc.h>

#include <iostream>
#include <cassert>
#include <iomanip>

#define assertm(exp, msg) assert(((void)msg, exp))

int main() {
    using T = int;
    constexpr std::size_t ROUNDS = 10;
    constexpr std::size_t PAGE_SIZE = 4096;
    constexpr std::size_t N = PAGE_SIZE/sizeof(T);

    T* ptrs[N];
    for (std::size_t r = 0; r < ROUNDS; ++r) {
        for (std::size_t i = 0; i < N; ++i) {
            auto ptr = reinterpret_cast<T*>(hse::malloc(sizeof(T)));
            *ptr = i;
            ptrs[i] = ptr;
        }
        for (std::size_t i = 0; i < N; ++i) {
            assertm(*ptrs[i] == i, "value was set to 1");
            hse::free(ptrs[i]);
        }
    }
    return 0;
}
