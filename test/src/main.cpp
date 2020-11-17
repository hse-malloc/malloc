#include <malloc.h>

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include <array>
#include <cstdint>
#include <span>

namespace {
    constexpr std::size_t BIG_NUMBER     = (1UL << 23U) - 1;
    constexpr std::size_t MEDIUM_NUMBER  = (1UL << 13U) + 1;
    constexpr std::size_t LESS_THAN_PAGE = 1UL << 11U;
    constexpr std::size_t SMALL_NUMBER   = 42;
} // namespace

template<typename T, std::size_t size>
void testArray(std::span<T, size> s) {
    for (auto &i : s) {
       i = '0';
    }
    for (auto &i : s) {
        REQUIRE(i == '0');
    }
}

TEST_CASE("malloc: 1 byte", "[malloc][free]") {
    auto *ptr = reinterpret_cast<std::uint8_t *>(hse::malloc(sizeof(std::uint8_t)));
    *ptr = 1;
    REQUIRE(*ptr == 1);
    hse::free(ptr);
}

TEST_CASE("malloc: array of bytes", "[malloc][free]") {
    auto *ptr = reinterpret_cast<std::uint8_t *>(hse::malloc(BIG_NUMBER * sizeof(std::uint8_t)));
    testArray(std::span{ptr, BIG_NUMBER});
    hse::free(ptr);
}

TEST_CASE("malloc: big object", "[malloc][free]") {
    struct S {
        std::span<std::uint8_t, BIG_NUMBER> data;
    };
    auto *ptr = reinterpret_cast<S *>(hse::malloc(sizeof(S)));
    testArray(ptr->data);
    hse::free(ptr);
}

TEST_CASE("malloc: array of big objects", "[malloc][free]") {
    struct S {
        std::span<std::uint8_t, MEDIUM_NUMBER> data;
    };

    std::span s{reinterpret_cast<S *>(hse::malloc(SMALL_NUMBER * sizeof(S))), SMALL_NUMBER};
    for (auto &i : s) {
        testArray(i.data);
    }
    hse::free(s.data());
}

TEST_CASE("calloc: zero arguments", "[calloc]") {
    REQUIRE(hse::calloc(0, 1) == nullptr);
    REQUIRE(hse::calloc(1, 0) == nullptr);
    REQUIRE(hse::calloc(0, 0) == nullptr);
}

TEST_CASE("calloc: array of small objects", "[calloc][free]") {
    auto *ptr = reinterpret_cast<std::uint8_t *>(hse::calloc(MEDIUM_NUMBER, sizeof(std::uint8_t)));
    testArray(std::span{ptr, MEDIUM_NUMBER});
    hse::free(ptr);
}

TEST_CASE("calloc: array of big objects", "[calloc][free]") {
    struct S {
        std::span<std::uint8_t, MEDIUM_NUMBER> data;
    };

    std::span s{reinterpret_cast<S *>(hse::calloc(SMALL_NUMBER, sizeof(S))), SMALL_NUMBER};
    for (auto &i : s) {
        testArray(i.data);
    }
    hse::free(s.data());
}

TEST_CASE("realloc: nullptr", "[realloc][free]") {
    auto *ptr = reinterpret_cast<std::uint8_t *>(hse::realloc(nullptr, MEDIUM_NUMBER * sizeof(std::uint8_t)));
    testArray(std::span{ptr, MEDIUM_NUMBER});
    hse::free(ptr);
}

TEST_CASE("realloc: bigger after small malloc", "[malloc][realloc][free][!mayfail]" ) {
    auto *ptr = reinterpret_cast<std::uint8_t *>(hse::malloc(SMALL_NUMBER * sizeof(std::uint8_t)));
    testArray(std::span{ptr, SMALL_NUMBER});
    ptr = reinterpret_cast<std::uint8_t *>(hse::realloc(ptr, MEDIUM_NUMBER));
    testArray(std::span{ptr, MEDIUM_NUMBER});
    hse::free(ptr);
}

TEST_CASE("realloc: bigger after big malloc", "[malloc][realloc][free]" ) {
    auto *ptr = reinterpret_cast<std::uint8_t *>(hse::malloc(MEDIUM_NUMBER * sizeof(std::uint8_t)));
    testArray(std::span{ptr, MEDIUM_NUMBER});
    ptr = reinterpret_cast<std::uint8_t *>(hse::realloc(ptr, BIG_NUMBER));
    testArray(std::span{ptr, BIG_NUMBER});
    hse::free(ptr);
}

TEST_CASE("realloc: smaller after big malloc", "[malloc][realloc][free]" ) {
    auto *ptr = reinterpret_cast<std::uint8_t *>(hse::malloc(BIG_NUMBER * sizeof(std::uint8_t)));
    testArray(std::span{ptr, BIG_NUMBER});
    ptr = reinterpret_cast<std::uint8_t *>(hse::realloc(ptr, MEDIUM_NUMBER));
    testArray(std::span{ptr, MEDIUM_NUMBER});
    hse::free(ptr);
}

TEST_CASE("realloc: smaller than page after malloc smaller than page", "[malloc][realloc][free]" ) {
    auto *ptr = reinterpret_cast<std::uint8_t *>(hse::malloc(LESS_THAN_PAGE * sizeof(std::uint8_t)));
    testArray(std::span{ptr, LESS_THAN_PAGE});
    ptr = reinterpret_cast<std::uint8_t *>(hse::realloc(ptr, SMALL_NUMBER));
    testArray(std::span{ptr, SMALL_NUMBER});
    hse::free(ptr);
}

TEST_CASE("realloc: smaller than page after malloc bigger than page", "[malloc][realloc][free]" ) {
    auto *ptr = reinterpret_cast<std::uint8_t *>(hse::malloc(MEDIUM_NUMBER * sizeof(std::uint8_t)));
    testArray(std::span{ptr, MEDIUM_NUMBER});
    ptr = reinterpret_cast<std::uint8_t *>(hse::realloc(ptr, SMALL_NUMBER));
    testArray(std::span{ptr, SMALL_NUMBER});
    hse::free(ptr);
}

TEST_CASE("aligned_alloc", "[aligned_alloc][free]") {
    constexpr std::size_t ALIGNMENT = 2UL << 6U;
    constexpr std::size_t SIZE      = ALIGNMENT * 4;
    auto *ptr = reinterpret_cast<std::uint8_t *>(hse::aligned_alloc(ALIGNMENT, SIZE * sizeof(std::uint8_t)));
    REQUIRE(reinterpret_cast<std::uintptr_t>(ptr) % ALIGNMENT == 0);
    testArray(std::span{ptr, SIZE});
    hse::free(ptr);
}

TEST_CASE("aligned_alloc: invalid aligment", "[aligned_alloc]") {
    REQUIRE(hse::aligned_alloc(16,  42) == nullptr);
    REQUIRE(hse::aligned_alloc(3,   42) == nullptr);
    REQUIRE(hse::aligned_alloc(67,  42) == nullptr);
    REQUIRE(hse::aligned_alloc(120, 42) == nullptr);
    REQUIRE(hse::aligned_alloc(0,   42) == nullptr);
}

TEST_CASE("aligned_alloc: big aligment", "[aligned_alloc][free]") {
    constexpr std::size_t ALIGNMENT = 2UL << 14U;
    constexpr std::size_t SIZE      = ALIGNMENT * 2;
    auto *ptr = reinterpret_cast<std::uint8_t *>(hse::aligned_alloc(ALIGNMENT, SIZE * sizeof(std::uint8_t)));
    REQUIRE(reinterpret_cast<std::uintptr_t>(ptr) % ALIGNMENT == 0);
    testArray(std::span{ptr, SIZE});
    hse::free(ptr);
}

TEST_CASE("aligned_alloc: small aligment", "[aligned_alloc][free]") {
    constexpr std::size_t ALIGNMENT = 2UL << 6U;
    constexpr std::size_t SIZE      = ALIGNMENT * 20;
    auto *ptr = reinterpret_cast<std::uint8_t *>(hse::aligned_alloc(ALIGNMENT, SIZE * sizeof(std::uint8_t)));
    REQUIRE(reinterpret_cast<std::uintptr_t>(ptr) % ALIGNMENT == 0);
    testArray(std::span{ptr, SIZE});
    hse::free(ptr);
}
