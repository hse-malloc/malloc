#include "malloc/malloc_implementation.h"

#include <catch2/catch_all.hpp>


TEST_CASE( "Simple malloc test", "[malloc][free]" ) {
    int* ptr = reinterpret_cast<int *>(hse::malloc(sizeof(int)));
    *ptr = 1;
    REQUIRE(*ptr == 1);
    hse::free(ptr);
}
