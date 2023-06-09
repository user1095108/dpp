#ifndef DPP_SQRT_HPP
# define DPP_SQRT_HPP
# pragma once

#include "dpp.hpp"

namespace dpp
{

namespace detail
{

template <typename T, typename E>
constexpr auto sqrt(intt::intt_concept auto m,
  typename dpp<T, E>::int_t e) noexcept
{
  using V = decltype(m);
  using int_t = decltype(e);

  if constexpr(V::words > 1)
  {
    constexpr int_t e0(
      intt::coeff<
        (1 * V::words / 2) *
        maxpow10e<typename V::value_type, decltype(e)>() - 1
      >()
    );

    e -= e0;
    m *= intt::coeff<pow(V(10), e0)>();
  }
  else
  {
    static_assert(1 == V::words);
    using U = std::make_unsigned_t<T>;

    constexpr int_t e0(intt::coeff<maxpow10e<U, decltype(e)>() - 1>());

    e -= e0;
    m *= intt::coeff<pow(V(10), e0)>();
  }

  //
  do
  {
    if (auto const tmp(intt::hwmul(m, 10)); (e & decltype(e)(1)) ||
      (ar::ucmp(tmp.v_, intt::coeff<V::max() / 5>().v_) <= 0))
    {
      --e;
      m = tmp;
    }
    else
    {
      break;
    }
  }
  while (ar::ucmp(m.v_, intt::coeff<V::max() / 5>().v_) <= 0);

  //
  if constexpr(ar::seqsqrt(m.v_); V::words > 1)
  {
    return dpp<T, E>(m, e / 2);
  }
  else
  {
    return dpp<T, E>(typename V::value_type(m), e / 2);
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

    return detail::sqrt<T, E>(V(intt::direct{}, U(a.sig())), a.exp());
  }
  else
  {
    using D = typename intt::intt<U, 1>::D;
    using V = intt::intt<D, 1>;

    return detail::sqrt<T, E>(V(intt::direct{}, D(a.sig())), a.exp());
  }
}

template <intt::intt_concept T, typename E>
constexpr auto sqrt(dpp<T, E> const& a) noexcept
{
  using U = std::make_unsigned_t<typename T::value_type>;
  using V = intt::intt<U, 2 * T::size()>;

  return detail::sqrt<T, E>(V(a.sig(), intt::direct{}), a.exp());
}

}

#endif // DPP_SQRT_HPP
