#include <malloc/malloc.h>

#include <iostream>
#include <cassert>
#include <array>
#include <iomanip>

#define assertm(exp, msg) assert(((void)msg, exp))

int main() {
    constexpr std::size_t N = 93;
    std::cout << "main start" << std::endl;
    std::cout 
        << "sizeof(std::size_t): " << sizeof(std::size_t) << std::endl
        << "sizeof(bool): " << sizeof(bool) << std::endl
        << "sizeof(void*): " << sizeof(void*) << std::endl;
    std::cout << "sizeof(T) = " << sizeof(int) << " bytes" << std::endl;
    int* ptrs[N][(4096-40-40)/sizeof(int*)+1];
    std::cout << "sizeof(ptrs) = " << sizeof(ptrs) << ", sizeof(ptrs[i]) = " << sizeof(ptrs[0]) << std::endl;
    for (int ii = 0; ii < 2; ++ii) {
        for (int i = 0; i < N; ++i) {
            // std::cout << i << std::endl;
            auto ptr = reinterpret_cast<int*>(hse::malloc(sizeof(ptrs[i])));
            std::cout << "[MALLOC] #" << i << ": " << std::hex << std::showbase << ptr
               << std::resetiosflags(std::ios_base::hex | std::ios_base::showbase) << std::endl; 
            *ptr = i;
            ptrs[i][0] = ptr;
            // ptrs[i] = ptr1;
            assertm(*ptr == i, "value was set to 1");
            // hse::free(ptr1);
        }

        for (int i = 0; i < N; ++i) {
            std::cout << "[FREE] #" << i << ": " << std::hex << std::showbase << ptrs[i][0]
                << std::resetiosflags(std::ios_base::hex | std::ios_base::showbase) << std::endl;
            // if (*ptrs[i] != i) {
            //     std::cerr << "!!!!!!!!!!!!!!!!! *ptrs[" << i << "] = " << *ptrs[i] << " != " << i << std::endl;
            // }
            // assertm(*ptrs[i] == i, "wrong value");
            hse::free(ptrs[i][0]);
        }
    }
    // for (auto&& ptr : ptrs)
    //     hse::free(ptr);
    // auto ptr1 = reinterpret_cast<int*>(hse::malloc(sizeof(int)));
    // *ptr1 = 1;
    // assertm(*ptr1 == 1, "value was set to 1");
    // hse::free(ptr1);


    return 0;
}
