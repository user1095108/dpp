#ifndef DPP_SQRT_HPP
# define DPP_SQRT_HPP
# pragma once

#include "dpp.hpp"

namespace dpp
{

namespace detail
{

template <typename T>
constexpr auto sqrt(intt::intt_type auto m, int_t e) noexcept
{ // no time for checks, write your own, if you need them
  using V = decltype(m);

  {
    constexpr auto e0(
      intt::coeff<
        (1 * V::words / 2) * maxpow10e<typename V::value_type>()
      >() - 1
    );

    m *= intt::coeff<pow<V, 10>(e0)>();
    e -= e0;
  }

  //
  for (; intt::ucompare(m, intt::coeff<V::max() / 10>()) <= 0;)
  {
    if (auto const tmp(intt::hwmul(10, m));
      !(e % 2) && intt::ucompare(tmp, intt::coeff<V::max() / 10>()) > 0)
    {
      break;
    }
    else
    {
      m = tmp;
      --e;
    }
  }

  //
  return dpp<T>(intt::seqsqrt(m), e / 2);
}

}

template <std::integral T>
constexpr auto sqrt(dpp<T> const& a) noexcept
{
  using U = typename std::make_unsigned<T>::type;
  using V = intt::intt<U, 2>;

  return detail::sqrt<T>(V(intt::direct{}, U(a.mantissa())), a.exponent());
}

template <intt::intt_type T>
constexpr auto sqrt(dpp<T> const& a) noexcept
{
  using U = typename std::make_unsigned<typename T::value_type>::type;
  using V = intt::intt<U, 2 * T::size()>;

  return detail::sqrt<T>(V(a.mantissa(), intt::direct{}), a.exponent());
}

}

#endif // DPP_SQRT_HPP
