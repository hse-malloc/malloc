#include "malloc/malloc_implementation.h"
#include <cstdint>

#include <catch2/catch_all.hpp>

namespace  {
    constexpr std::size_t bigNumber = 93547289;
    constexpr std::size_t notSoBigNumber = 5000;
    constexpr std::size_t lessThanPage = 2048;
    constexpr std::size_t smallNumber = 42;
}

TEST_CASE( "Simple malloc test", "[malloc][free]" ) {
    int* ptr = reinterpret_cast<int *>(hse::malloc(sizeof(int)));
    *ptr = 1;
    REQUIRE(*ptr == 1);
    hse::free(ptr);
}

TEST_CASE( "Allocatig big array of little objects with malloc", "[malloc][free]" ) {
    char* ptr = reinterpret_cast<char *>(hse::malloc(sizeof (char[bigNumber])));
    for(std::size_t i = 0; i < bigNumber; ++i){
        ptr[i] = '0';
        REQUIRE(ptr[i] == '0');
    }
    hse::free(ptr);
}

TEST_CASE( "Allocatig big single object with malloc", "[malloc][free]" ) {

    struct bigObject
    {
        char data[bigNumber];
    };

    bigObject* ptr = reinterpret_cast<bigObject *>(hse::malloc(sizeof(bigObject)));
    ptr->data[13] = '0';
    REQUIRE(ptr->data[13] == '0');
    hse::free(ptr);
}

TEST_CASE( "Allocatig array of big objects with malloc", "[malloc][free]" ) {
    struct bigObject
    {
        char data[notSoBigNumber];
    };
    bigObject* ptr = reinterpret_cast<bigObject *>(hse::malloc(sizeof (bigObject[smallNumber])));
    for(std::size_t i = 0; i < smallNumber; ++i){
        ptr[i].data[0] = '0';
        REQUIRE(ptr[i].data[0] == '0');
    }
    hse::free(ptr);
}


TEST_CASE( "Calloc with zero arguments", "[calloc]" ) {
    REQUIRE(hse::calloc(0, 11) == nullptr);
    REQUIRE(hse::calloc(11, 0) == nullptr);
    REQUIRE(hse::calloc(0,  0) == nullptr);
}


TEST_CASE( "Allocatig array of small objects with calloc", "[calloc][free]" ) {
    char* ptr = reinterpret_cast<char *>(hse::calloc(notSoBigNumber, sizeof (char)));
    for(std::size_t i = 0; i < notSoBigNumber; ++i){
        ptr[i] = '0';
        REQUIRE(ptr[i] == '0');
    }
    hse::free(ptr);
}

TEST_CASE( "Allocatig array of big objects with calloc", "[calloc][free]" ) {
    struct bigObject
    {
        char data[notSoBigNumber];
    };
    bigObject* ptr = reinterpret_cast<bigObject *>(hse::calloc(smallNumber, sizeof (bigObject)));
    for(std::size_t i = 0; i < smallNumber; ++i){
        ptr[i].data[0] = '0';
        REQUIRE(ptr[i].data[0] == '0');
    }
    hse::free(ptr);
}


TEST_CASE( "Realloc with nullptr", "[realloc][free]" ) {
    char* ptr = reinterpret_cast<char *>(hse::realloc(nullptr, notSoBigNumber));
    for(std::size_t i = 0; i < notSoBigNumber; ++i){
        ptr[i] = '0';
        REQUIRE(ptr[i] == '0');
    }
    hse::free(ptr);
}

TEST_CASE( "Realloc bigger after small malloc", "[malloc][realloc][free][!mayfail]" ) {
    char* ptr = reinterpret_cast<char *>(hse::malloc(smallNumber));
    for(std::size_t i = 0; i < smallNumber; ++i){
        ptr[i] = '0';
        REQUIRE(ptr[i] == '0');
    }
    ptr = reinterpret_cast<char *>(hse::realloc(ptr, notSoBigNumber));
    for(std::size_t i = 0; i < notSoBigNumber; ++i){
        ptr[i] = '1';
        REQUIRE(ptr[i] == '1');
    }
    hse::free(ptr);
}

TEST_CASE( "Realloc bigger after big malloc", "[malloc][realloc][free]" ) {
    char* ptr = reinterpret_cast<char *>(hse::malloc(notSoBigNumber));
    for(std::size_t i = 0; i < notSoBigNumber; ++i){
        ptr[i] = '0';
        REQUIRE(ptr[i] == '0');
    }
    ptr = reinterpret_cast<char *>(hse::realloc(ptr, bigNumber));
    for(std::size_t i = 0; i < bigNumber; ++i){
        ptr[i] = '1';
        REQUIRE(ptr[i] == '1');
    }
    hse::free(ptr);
}


TEST_CASE( "Realloc smaller after big malloc", "[malloc][realloc][free]" ) {
    char* ptr = reinterpret_cast<char *>(hse::malloc(bigNumber));
    for(std::size_t i = 0; i < bigNumber; ++i){
        ptr[i] = '0';
        REQUIRE(ptr[i] == '0');
    }
    ptr = reinterpret_cast<char *>(hse::realloc(ptr, notSoBigNumber));
    for(std::size_t i = 0; i < notSoBigNumber; ++i){
        ptr[i] = '1';
        REQUIRE(ptr[i] == '1');
    }
    hse::free(ptr);
}

TEST_CASE( "Realloc smaller than page after malloc smaller than page", "[malloc][realloc][free]" ) {
    char* ptr = reinterpret_cast<char *>(hse::malloc(lessThanPage));
    for(std::size_t i = 0; i < lessThanPage; ++i){
        ptr[i] = '0';
        REQUIRE(ptr[i] == '0');
    }
    ptr = reinterpret_cast<char *>(hse::realloc(ptr, smallNumber));
    for(std::size_t i = 0; i < smallNumber; ++i){
        ptr[i] = '1';
        REQUIRE(ptr[i] == '1');
    }
    hse::free(ptr);
}

TEST_CASE( "Realloc smaller than page after malloc bigger than page", "[malloc][realloc][free]" ) {
    char* ptr = reinterpret_cast<char *>(hse::malloc(notSoBigNumber));
    for(std::size_t i = 0; i < notSoBigNumber; ++i){
        ptr[i] = '0';
        REQUIRE(ptr[i] == '0');
    }
    ptr = reinterpret_cast<char *>(hse::realloc(ptr, smallNumber));
    for(std::size_t i = 0; i < smallNumber; ++i){
        ptr[i] = '1';
        REQUIRE(ptr[i] == '1');
    }
    hse::free(ptr);
}

TEST_CASE( "Aligned_alloc simple test", "[aligned_alloc][free]" ) {
    char* ptr = reinterpret_cast<char *>(hse::aligned_alloc(64, 64));
    for(std::size_t i = 0; i < 64; ++i){
        ptr[i] = '0';
        REQUIRE(ptr[i] == '0');
    }
    hse::free(ptr);
}


TEST_CASE( "Aligned_alloc with invalid aligment", "[aligned_alloc]" ) {
    REQUIRE(hse::aligned_alloc(16, smallNumber)==nullptr);
    REQUIRE(hse::aligned_alloc(3, smallNumber)==nullptr);
    REQUIRE(hse::aligned_alloc(67, smallNumber)==nullptr);
    REQUIRE(hse::aligned_alloc(120, smallNumber)==nullptr);
    REQUIRE(hse::aligned_alloc(0, smallNumber)==nullptr);
}

TEST_CASE( "Aligned_alloc with big aligment", "[aligned_alloc][free]" ) {
    char* ptr = reinterpret_cast<char *>(hse::aligned_alloc(4096*4, 4096*4));
    hse::free(ptr);
}

TEST_CASE( "Aligned_alloc with small aligment", "[aligned_alloc][free]" ) {
    char* ptr = reinterpret_cast<char *>(hse::aligned_alloc(64, 10248*64));
    hse::free(ptr);
}
