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

  if constexpr(V::words > 1)
  {
    constexpr auto e0(
      intt::coeff<
        (1 * V::words / 2) * maxpow10e<typename V::value_type>() - 1
      >()
    );

    e -= e0;
    m *= intt::coeff<pow<V, 10>(e0)>();
  }
  else
  {
    static_assert(1 == V::words);

    using U = typename V::value_type;
    using H = std::conditional_t<
      std::is_same_v<U, std::uint64_t>,
      std::uint32_t,
      std::conditional_t<
        std::is_same_v<U, std::uint32_t>,
        std::uint16_t,
        std::uint8_t
      >
    >;

    constexpr auto e0(intt::coeff<maxpow10e<H>() - 1>());

    e -= e0;
    m *= intt::coeff<pow<V, 10>(e0)>();
  }

  //
  do
  {
    if (auto const tmp(intt::hwmul(10, m));
      (e % 2) || (intt::ucompare(tmp, intt::coeff<V::max() / 5>()) <= 0))
    {
      --e;
      m = tmp;
    }
    else
    {
      break;
    }
  }
  while (intt::ucompare(m, intt::coeff<V::max() / 5>()) <= 0);

  //
  if constexpr(V::words > 1)
  {
    return dpp<T>(intt::seqsqrt(m), e / 2);
  }
  else
  {
    return dpp<T>(typename V::value_type(intt::seqsqrt(m)), e / 2);
  }
}

}

template <std::integral T>
constexpr auto sqrt(dpp<T> const& a) noexcept
{
  using U = typename std::make_unsigned<T>::type;

  if constexpr(std::is_same_v<U, std::uint64_t>)
  {
    using V = intt::intt<U, 2>;

    return detail::sqrt<T>(V(intt::direct{}, U(a.mantissa())), a.exponent());
  }
  else
  {
    using D = std::conditional_t<
      std::is_same_v<U, std::uint8_t>,
      std::uint16_t,
      std::conditional_t<
        std::is_same_v<U, std::uint16_t>,
        std::uint32_t,
        std::conditional_t<
          std::is_same_v<U, std::uint32_t>,
          std::uint64_t,
          void
        >
      >
    >;
    using V = intt::intt<D, 1>;

    return detail::sqrt<T>(V(intt::direct{}, D(a.mantissa())), a.exponent());
  }
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
