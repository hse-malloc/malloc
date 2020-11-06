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
            std::cout << "[MALLOC] " << i << ": ";
            auto ptr = reinterpret_cast<T*>(std::malloc(sizeof(T)));
            std::cout << ptr << std::endl;
            *ptr = i;
            ptrs[i] = ptr;
        }
        for (std::size_t i = 0; i < N; ++i) {
            std::cout << "[FREE] "<< i << ": " << ptrs[i] << std::endl;
            assertm(*ptrs[i] == i, "value was set to 1");
            std::free(ptrs[i]);
        }
    }

    for (std::size_t r = 0; r < ROUNDS; ++r) {
        for (std::size_t i = 0; i < N; ++i) {
            std::cout << "[CALLOC] " << i << ": ";
            auto ptr = reinterpret_cast<T*>(std::calloc(1, sizeof(T)));
            std::cout << ptr << "; value: " << *ptr << std::endl;
            *ptr = i;
            ptrs[i] = ptr;
        }
        for (std::size_t i = 0; i < N; ++i) {
            std::cout << "[FREE] "<< i << ": " << ptrs[i] << std::endl;
            assertm(*ptrs[i] == i, "value was set to 1");
            std::free(ptrs[i]);
        }
    }
    return 0;
}
