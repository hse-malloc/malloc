#ifndef MATH_H
#define MATH_H

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
}

#endif // MATH_H
