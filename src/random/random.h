#ifndef RANDOM_H
#define RANDOM_H

#include "math.h"

#include <cstdint>
#include <type_traits>
#include <limits>
#include <chrono>

namespace hse {


template <typename _IntType =  std::uint64_t,  typename = std::enable_if<std::is_integral<_IntType>::value>,
          _IntType k1=8, _IntType k2=69069, _IntType b=313>
class squared_congruential_generator
{
    using result_type = _IntType;
    // init prng value
    result_type __start;

    // peek the next value and do not chage state
    result_type next(result_type start) const noexcept { return k1*start*start + k2*start + b; }

public:

    squared_congruential_generator() noexcept : __start(0) {}

    // inititalize generator with statr value
    squared_congruential_generator(result_type token) noexcept : __start(token) {}

    // change state and return the next value
    result_type operator()() noexcept { __start = next(__start); return next(__start); }

    // min and max pissible values
    result_type min() const noexcept { return std::numeric_limits<_IntType>::min(); }
    result_type max() const noexcept { return std::numeric_limits<_IntType>::max(); }
    std::size_t range() const noexcept { return max() - min(); }
};


template<typename _IntType = std::uint64_t, typename = std::enable_if<std::is_integral<_IntType>::value>>
  class uniform_int_distribution
  {
    _IntType __min;
    _IntType __max;

  public:
    using result_type = _IntType;

    uniform_int_distribution() {}

    // initialize min and max value of distribution
    // if the _max<=_min then generaor always renturns _min value
    explicit uniform_int_distribution(_IntType _min = std::numeric_limits<_IntType>::min(),
                                         _IntType _max = std::numeric_limits<_IntType>::max()) noexcept
                                         : __min(_min), __max(_min<_max?_max:_min){}

    // min and max values of generator
    result_type min() const noexcept { return __min; }
    result_type max() const noexcept { return __max; }
    std::size_t range() const noexcept { return __max - __min; }

    // returning random number from prng
    template<typename _UniformRandomNumberGenerator>
    result_type  operator()(_UniformRandomNumberGenerator& __prng) noexcept
    {   return min() + __prng() * range()/__prng.range(); }



  };


  namespace sch = std::chrono;

  // function that converts time_point to int
  template <typename Clock, typename IntType = std::size_t>
  IntType timeToInt(sch::time_point<Clock> time) noexcept
  {
      auto epoch = sch::time_point_cast<sch::milliseconds>(time).time_since_epoch();
      auto value = sch::duration_cast<sch::milliseconds>(epoch);
      return static_cast<IntType>(value.count());
  }

  // function that returns current time in integer value
  template <typename Clock=sch::steady_clock, typename IntType = std::size_t>
  IntType timeToInt() noexcept { return timeToInt(Clock::now()); }

  typedef squared_congruential_generator<>  sc69069_t;


}


#endif // RANDOM_H
