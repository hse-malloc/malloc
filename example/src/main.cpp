#include <malloc.h>

#include <iostream>

int main() {
    int *ptr = reinterpret_cast<int*>(mymalloc::mem_alloc(sizeof(int)));

    std::cout << "allocated " << sizeof(int) << " bytes at " << std::hex << std::showbase << ptr
        << "\nuninitialized int: " << *ptr << std::endl;

    *ptr = 1;

    std::cout << "value was set to 1: " << *ptr << std::endl;

    mymalloc::mem_free(ptr);

    std::cout << "freed at " << std::hex << std::showbase << ptr << std::endl;
    return 0;
}
