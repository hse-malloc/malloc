#ifndef MATH_H
#define MATH_H

#include <cstdint>
#include <type_traits>

namespace hse::math {
	// roundDown returns maximum multiple of given multiplier,
	// which does not exceed given num
	template<typename T, typename = std::enable_if<std::is_integral<T>::value>>
	constexpr T roundDown(T num, T multiplier) noexcept {
		return num & (-multiplier);
	}

	// roundDown returns munumum multiple of given multiplier,
	// which is not less than given num
	template<typename T, typename = std::enable_if<std::is_integral<T>::value>>
	constexpr T roundUp(T num, T multiplier) noexcept {
		return roundDown<T>(num + multiplier - 1, multiplier);	
	}

	// nthBit returns nth bit of given num, starting from zero
	template<typename T, typename = std::enable_if<std::is_integral<T>::value>>
	constexpr bool nthBit(T num, std::uint8_t n) noexcept {
		return (num >> n) & 1;
	}
	
	// setNethBit sets nth bit of given num to 1 starting from zero
	template<typename T, typename = std::enable_if<std::is_integral<T>::value>>
	constexpr T setNthBit(T num, std::uint8_t n) noexcept {
		return num | (1 << n);
	}

	// setNethBit sets nth bit of given num to 1 starting from zero
	template<typename T, typename = std::enable_if<std::is_integral<T>::value>>
	constexpr T clearNthBit(T num, std::uint8_t n) noexcept {
		return num & ~(1 << n);
	}


	// toogleNthBit inverts nth bit of given number starting from zero
	template<typename T, typename = std::enable_if<std::is_integral<T>::value>>
	constexpr T toogleNthBit(T num, std::uint8_t n) noexcept {
		return num ^ (1 << n);
	}
}

#endif // MATH_H
