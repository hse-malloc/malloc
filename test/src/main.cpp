#include <malloc/malloc.h>

#include <iostream>
#include <cassert>

#define assertm(exp, msg) assert(((void)msg, exp))

int main() {
    auto ptr = reinterpret_cast<int*>(mymalloc::mem_alloc(sizeof(int)));
    *ptr = 1;
    assertm(*ptr == 1, "value was set to 1");
    mymalloc::mem_free(ptr);
    return 0;
}
