#ifndef MATH_H
#define MATH_H

#include "concepts/numbers.h"

#include <cstdint>

namespace hse::math {

template<UnsignedIntegral T>
constexpr bool isPowerOf2(T x) noexcept {
    return x != 0 && ((x & (x - 1)) == 0);
}

// roundDown returns maximum multiple of given multiplier,
// which does not exceed given num
template<Integral T, Integral U>
constexpr T roundDown(T num, U multiplier) noexcept {
    return num & (-multiplier);
}

// roundUp returns munumum multiple of given multiplier,
// which is not less than given num
template<Integral T, Integral U>
constexpr T roundUp(T num, U multiplier) noexcept {
    return roundDown(num + multiplier - 1, multiplier);
}

// nthBit returns nth bit of given num, starting from zero
template<Integral T>
constexpr bool nthBit(T num, std::uint8_t n) noexcept {
    return (num >> n) & 1;
}

// setNethBit sets nth bit of given num to 1 starting from zero
template<Integral T>
constexpr T setNthBit(T num, std::uint8_t n) noexcept {
    return num | (1U << n);
}

// clearNethBit sets nth bit of given num to 0 starting from zero
template<Integral T>
constexpr T clearNthBit(T num, std::uint8_t n) noexcept {
    return num & ~(1U << n);
}

// toogleNthBit inverts nth bit of given number starting from zero
template<Integral T>
constexpr T toogleNthBit(T num, std::uint8_t n) noexcept {
    return num ^ (1U << n);
}

} // namespace hse::math

#endif // MATH_H
