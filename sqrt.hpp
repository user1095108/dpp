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
{
  using V = decltype(m);

  m *= intt::coeff<
      pow<V, 10>((V::words - 1) * maxpow10e<typename V::value_type>())
    >();
  e -= intt::coeff<(V::words - 1) * maxpow10e<typename V::value_type>()>();

  {
    constexpr auto k(intt::coeff<V(10)>());

    for (; m <= intt::coeff<V::max() / k>(); m *= k, --e);
    if (e % 2) { ++e; m /= k; }
  }

  return dpp<T>(intt::seqsqrt(m), e / 2);
}

}

template <std::integral T>
constexpr auto sqrt(dpp<T> const& a) noexcept
{
  using U = typename std::make_unsigned<T>::type;
  using V = intt::intt<U, 3>;

  return detail::sqrt<T>(V(intt::direct{}, U(a.mantissa())), a.exponent());
}

template <intt::intt_type T>
constexpr auto sqrt(dpp<T> const& a) noexcept
{
  using U = typename std::make_unsigned<typename T::value_type>::type;
  using V = intt::intt<U, 3 * T::size()>;

  return detail::sqrt<T>(V(a.mantissa(), intt::direct{}), a.exponent());
}

}

#endif // DPP_SQRT_HPP
