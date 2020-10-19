#ifndef MATH_H
#define MATH_H

#include <type_traits>

namespace hse::math {
	// roundDown returns maximum multiple of given multiplier,
	// which does not exceed given num
	template<typename T, typename = std::enable_if<std::is_integral<T>::value>>
	T roundDown(T num, T multiplier) noexcept {
		if (multiplier == 0)
			return num;
		T remainder = num % multiplier;
		if (remainder == 0)
			return num;
		return num - remainder;
	}

	// roundDown returns munumum multiple of given multiplier,
	// which is not less than given num
	template<typename T, typename = std::enable_if<std::is_integral<T>::value>>
	T roundUp(T num, T multiplier) noexcept {
		T down = roundDown<T>(num, multiplier);
		if (down == num)
			return num;
		return down + multiplier;
	}
}

#endif // MATH_H
