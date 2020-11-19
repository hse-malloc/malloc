#include <malloc.h>

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include <array>
#include <cstdint>
#include <span>

namespace {
    // all constans are not aligned
    constexpr std::size_t BIG_NUMBER     = (1UL << 23U) + 1;
    constexpr std::size_t MEDIUM_NUMBER  = (1UL << 13U) + 1;
    constexpr std::size_t LESS_THAN_PAGE = (1UL << 11U) + 1;
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
        std::array<std::uint8_t, BIG_NUMBER> data;
    };
    auto *ptr = reinterpret_cast<S *>(hse::malloc(sizeof(S)));
    testArray(std::span{ptr->data});
    hse::free(ptr);
}

TEST_CASE("malloc: array of big objects", "[malloc][free]") {
    struct S {
        std::array<std::uint8_t, MEDIUM_NUMBER> data;
    };

    std::span s{reinterpret_cast<S *>(hse::malloc(SMALL_NUMBER * sizeof(S))), SMALL_NUMBER};
    for (auto &i : s) {
        testArray(std::span{i.data});
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
        std::array<std::uint8_t, MEDIUM_NUMBER> data;
    };

    std::span s{reinterpret_cast<S *>(hse::calloc(SMALL_NUMBER, sizeof(S))), SMALL_NUMBER};
    for (auto &i : s) {
        testArray(std::span{i.data});
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

TEST_CASE("multiple usage: aligned_alloc with small aligment, "
                          "less than page malloc, "
                          "aligned_alloc with big aligment", "[aligned_alloc][malloc][free]") {
    constexpr std::size_t SMALL_ALIGNMENT = 2UL << 6U;
    constexpr std::size_t SMALL_SIZE      = SMALL_ALIGNMENT * 2;
    constexpr std::size_t   BIG_ALIGNMENT = 2UL << 14U;
    constexpr std::size_t   BIG_SIZE      = BIG_ALIGNMENT * 2;

    auto *ptr_1 = reinterpret_cast<std::uint8_t *>(hse::aligned_alloc(SMALL_ALIGNMENT, SMALL_SIZE * sizeof(std::uint8_t)));
    REQUIRE(reinterpret_cast<std::uintptr_t>(ptr_1) % SMALL_ALIGNMENT == 0);
    testArray(std::span{ptr_1, SMALL_SIZE});

    auto *ptr_2 = reinterpret_cast<std::uint8_t *>(hse::malloc(LESS_THAN_PAGE * sizeof(std::uint8_t)));
    testArray(std::span{ptr_2, LESS_THAN_PAGE});


    auto *ptr_3 = reinterpret_cast<std::uint8_t *>(hse::aligned_alloc(BIG_ALIGNMENT, BIG_SIZE * sizeof(std::uint8_t)));
    REQUIRE(reinterpret_cast<std::uintptr_t>(ptr_3) % BIG_ALIGNMENT == 0);
    testArray(std::span{ptr_3, SMALL_SIZE});

    hse::free(ptr_1);
    hse::free(ptr_2);
    hse::free(ptr_3);
}

TEST_CASE("multiple usage: aligned_alloc with small aligment + realloc to less than page, "
                          "aligned_alloc with big aligment", "[aligned_alloc][realloc][free]") {
    constexpr std::size_t ALIGNMENT = 2UL << 6U;
    constexpr std::size_t SIZE      = ALIGNMENT * 2;

    auto *ptr_1 = reinterpret_cast<std::uint8_t *>(hse::aligned_alloc(ALIGNMENT, SIZE * sizeof(std::uint8_t)));
    REQUIRE(reinterpret_cast<std::uintptr_t>(ptr_1) % ALIGNMENT == 0);
    testArray(std::span{ptr_1, SIZE});

    ptr_1 = reinterpret_cast<std::uint8_t *>(hse::realloc(ptr_1, LESS_THAN_PAGE * sizeof(std::uint8_t)));
    testArray(std::span{ptr_1, LESS_THAN_PAGE});


    auto *ptr_2 = reinterpret_cast<std::uint8_t *>(hse::aligned_alloc(ALIGNMENT, SIZE * sizeof(std::uint8_t)));
    REQUIRE(reinterpret_cast<std::uintptr_t>(ptr_2) % ALIGNMENT == 0);
    testArray(std::span{ptr_2, SIZE});

    hse::free(ptr_1);
    hse::free(ptr_2);
}

TEST_CASE("multiple usage: aligned_alloc with small aligment, "
                          "small malloc,"
                          "realloc 1st ptr to less than page, "
                          "free 2nd ptr"
                          "aligned_alloc with medium aligment, "
                          "big malloc, "
                          "aligned_alloc with big aligment [aligned_alloc][malloc][realloc][free]") {
    constexpr std::size_t  SMALL_ALIGNMENT = 2UL << 6U;
    constexpr std::size_t  SMALL_SIZE      = SMALL_ALIGNMENT * 2;
    constexpr std::size_t MEDIUM_ALIGNMENT = 2UL << 10U;
    constexpr std::size_t MEDIUM_SIZE      = MEDIUM_ALIGNMENT * 2;
    constexpr std::size_t   BIG_ALIGNMENT  = 2UL << 14U;
    constexpr std::size_t   BIG_SIZE       = BIG_ALIGNMENT * 2;

    auto *ptr_1 = reinterpret_cast<std::uint8_t *>(hse::aligned_alloc(SMALL_ALIGNMENT, SMALL_SIZE * sizeof(std::uint8_t)));
    REQUIRE(reinterpret_cast<std::uintptr_t>(ptr_1) % SMALL_ALIGNMENT == 0);
    testArray(std::span{ptr_1, SMALL_ALIGNMENT});

    auto *ptr_2 = reinterpret_cast<std::uint8_t *>(hse::malloc(SMALL_NUMBER * sizeof(std::uint8_t)));
    testArray(std::span{ptr_2, SMALL_NUMBER});

    ptr_1 = reinterpret_cast<std::uint8_t *>(hse::realloc(ptr_1, LESS_THAN_PAGE * sizeof(std::uint8_t)));
    testArray(std::span{ptr_1, LESS_THAN_PAGE});

    hse::free(ptr_2);

    auto *ptr_3 = reinterpret_cast<std::uint8_t *>(hse::aligned_alloc(MEDIUM_ALIGNMENT, MEDIUM_SIZE * sizeof(std::uint8_t)));
    REQUIRE(reinterpret_cast<std::uintptr_t>(ptr_3) % MEDIUM_ALIGNMENT == 0);
    testArray(std::span{ptr_3, MEDIUM_SIZE});

    auto *ptr_4 = reinterpret_cast<std::uint8_t *>(hse::malloc(BIG_NUMBER * sizeof(std::uint8_t)));
    testArray(std::span{ptr_4, BIG_NUMBER});

    auto *ptr_5 = reinterpret_cast<std::uint8_t *>(hse::aligned_alloc(BIG_ALIGNMENT, BIG_SIZE * sizeof(std::uint8_t)));
    REQUIRE(reinterpret_cast<std::uintptr_t>(ptr_5) % BIG_ALIGNMENT == 0);
    testArray(std::span{ptr_5, BIG_SIZE});

    hse::free(ptr_1);
    // already freed ptr_2
    hse::free(ptr_3);
    hse::free(ptr_4);
    hse::free(ptr_5);
}
