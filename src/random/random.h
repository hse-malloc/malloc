#ifndef RANDOM_H
#define RANDOM_H

#include "concepts/numbers.h"
#include "math/math.h"

#include <chrono>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace hse {

template <Integral IntType = std::uint64_t, IntType k1 = 8, IntType k2 = 69069, IntType b = 313>
class squared_congruential_generator {
    using result_type = IntType;
    // init prng value
    result_type __start;

    // peek the next value and do not chage state
    result_type next(result_type start) const noexcept {
        return k1 * start * start + k2 * start + b;
    }

  public:
    squared_congruential_generator() noexcept : __start(17) {}

    // inititalize generator with statr value
    squared_congruential_generator(result_type token) noexcept
        : __start(token) {}

    // change state and return the next value
    result_type operator()() noexcept {
        __start = next(__start);
        return next(__start);
    }

    // min and max possible values
    result_type min() const noexcept { return 0; }
    result_type max() const noexcept {
        return std::numeric_limits<IntType>::max();
    }
};

template<typename Fn, typename T = std::uint64_t>
concept PRNG = std::is_nothrow_invocable_r_v<T, Fn>;

template <Integral IntType = std::uint64_t>
class uniform_int_distribution {
    IntType __min;
    IntType __max;

  public:
    using result_type = IntType;

    // initialize min and max value of distribution
    // if the _max<=_min then generaor always renturns _min value
    explicit uniform_int_distribution(
        IntType _min = 0,
        IntType _max = std::numeric_limits<IntType>::max()) noexcept
        : __min(_min), __max(_min < _max ? _max : _min) {}

    // min and max values of generator
    result_type min() const noexcept { return __min; }
    result_type max() const noexcept { return __max; }

    // returning random number from prng
    template <PRNG<result_type> UniformRandomNumberGenerator>
    result_type operator()(UniformRandomNumberGenerator &__prng) noexcept {
        return min() + __prng() % (max() - min());
    }
};

namespace sch = std::chrono;

// function that converts time_point to int
template <typename Clock, Integral IntType = std::size_t>
IntType timeToInt(sch::time_point<Clock> time) noexcept {
    auto epoch =
        sch::time_point_cast<sch::milliseconds>(time).time_since_epoch();
    auto value = sch::duration_cast<sch::milliseconds>(epoch);
    return static_cast<IntType>(value.count());
}

// function that returns current time in integer value
template <typename Clock = sch::steady_clock, typename IntType = std::size_t>
IntType timeToInt() noexcept {
    return timeToInt(Clock::now());
}

typedef squared_congruential_generator<> sc69069_t;

} // namespace hse

#endif // RANDOM_H
