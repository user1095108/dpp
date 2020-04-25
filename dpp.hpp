#ifndef DPP_HPP
# define DPP_HPP
# pragma once

#include <cstdint>

#include <type_traits>

template <unsigned M, unsigned E>
class dpp
{

using value_type = std::conditional_t<M + E == 64, std::uint64_t,
  std::conditional_t<M + E == 32, std::uint32_t,
    std::conditional_t<M + E == 16, std::uint16_t, void>
  >
>;

struct 
{
  value_type m:M;
  value_type e:E;
} value;

public:
};

using dec64 = dpp<56, 8>;
using dec32 = dpp<28, 4>;
using dec16 = dpp<14, 2>;

#endif // DPP_HPP
