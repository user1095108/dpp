#ifndef DPP_SQRT_HPP
# define DPP_SQRT_HPP
# pragma once

#include "dpp.hpp"

namespace dpp
{

namespace detail
{

template <typename T, typename E>
constexpr auto sqrt(intt::is_intt auto m,
  typename dpp<T, E>::exp2_t e) noexcept
{
  using V = decltype(m);
  using exp2_t = decltype(e);

  if constexpr(V::words > 1)
  {
    constexpr exp2_t e0(ar::coeff<(V::words - 1) *
      maxpow10e<typename V::value_type, decltype(e)>() - 1>());

    e -= e0;
    m *= ar::coeff<pow(V(10), e0)>();
  }
  else
  { // V::words == 1, m is doubled
    using U = std::make_unsigned_t<T>;

    constexpr exp2_t e0(ar::coeff<maxpow10e<U, decltype(e)>() - 1>());

    e -= e0;
    m *= ar::coeff<pow(V(10), e0)>();
  }

  //
  do
  { // 2 * V::max / 10 = V::max() / 5
    if (auto const tmp(ar::hwmul(m.v_, 10)); (e & decltype(e)(1)) ||
      (ar::ucmp(tmp, ar::coeff<V::max() / 5>().v_) <= 0))
    {
      --e;
      ar::copy(m.v_, tmp);
    }
    else
    {
      break;
    }
  }
  while (ar::ucmp(m.v_, ar::coeff<V::max() / 5>().v_) <= 0);

  //
  if constexpr(ar::seqsqrt(m.v_); V::words > 1)
  {
    return dpp<T, E>(m, e / 2);
  }
  else
  {
    return dpp<T, E>(m.v_.front(), e / 2);
  }
}

}

template <std::integral T, typename E>
constexpr auto sqrt(dpp<T, E> const& a) noexcept
{
  using U = std::make_unsigned_t<T>;

  if constexpr(std::is_same_v<U, std::uintmax_t>)
  {
    using V = intt::intt<U, 2>;

    return detail::sqrt<T, E>(V(intt::direct, U(a.sig())), a.exp());
  }
  else
  {
    using D = ar::D<U>;
    using V = intt::intt<D, 1>;

    return detail::sqrt<T, E>(V(intt::direct, D(a.sig())), a.exp());
  }
}

template <intt::is_intt T, typename E>
constexpr auto sqrt(dpp<T, E> const& a) noexcept
{
  using U = typename T::value_type;
  using V = intt::intt<U, 2 * T::size()>;

  return detail::sqrt<T, E>(V(intt::direct, a.sig()), a.exp());
}

}

#endif // DPP_SQRT_HPP
