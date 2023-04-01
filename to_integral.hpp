#ifndef DPP_TO_INTEGRAL_HPP
# define DPP_TO_INTEGRAL_HPP
# pragma once

#include "dpp.hpp"

namespace dpp
{

template <typename U = std::intmax_t, typename T, typename E>
constexpr auto to_integral(dpp<T, E> const& a) noexcept
{
  if (isnan(a))
  {
    return std::pair(U{}, true);
  }
  else
  {
    T m;

    if (auto const e(a.exponent()); e <= E{})
    {
      m = T(a);

      if ((m < intt::coeff<detail::min_v<U>>()) ||
        (m > intt::coeff<detail::max_v<U>>()))
      {
        return std::pair(U{}, true);
      }
    }
    else
    {
      m = a.mantissa();

      do
      {
        if ((m >= intt::coeff<detail::min_v<U> / 10>()) &&
          (m <= intt::coeff<detail::max_v<U> / 10>()))
        {
          m *= 10;
        }
        else
        {
          return std::pair(U{}, true);
        }
      }
      while (--e);
    }

    return std::pair(U(m), false);
  }
}

}

#endif // DPP_TO_INTEGRAL_HPP
