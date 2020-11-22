#ifndef CONCEPTS_NUMBERS_H
#define CONCEPTS_NUMBERS_H

#include <type_traits>

namespace hse {

template<typename T>
concept Integral = std::is_integral_v<T>;

template<typename T>
concept UnsignedIntegral = Integral<T> && std::is_unsigned_v<T>;

} // namespace hse

#endif // CONCEPTS_NUMBERS_H
