#ifndef DPP_SQRT_HPP
# define DPP_SQRT_HPP
# pragma once

#include "dpp.hpp"

namespace dpp
{

namespace detail
{

template <typename T, typename E>
constexpr auto sqrt(intt::intt_type auto m,
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
    m *= intt::coeff<pow<V, 10>(e0)>();
  }
  else
  {
    static_assert(1 == V::words);
    using U = std::make_unsigned_t<T>;

    constexpr int_t e0(intt::coeff<maxpow10e<U, decltype(e)>() - 1>());

    e -= e0;
    m *= intt::coeff<pow<V, 10>(e0)>();
  }

  //
  do
  {
    if (auto const tmp(intt::hwmul(m, 10));
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
    return dpp<T, E>(intt::seqsqrt(m), e / 2);
  }
  else
  {
    return dpp<T, E>(typename V::value_type(intt::seqsqrt(m)), e / 2);
  }
}

}

template <std::integral T, typename E>
constexpr auto sqrt(dpp<T, E> const& a) noexcept
{
  using U = std::make_unsigned_t<T>;

  if constexpr(std::is_same_v<U, std::uint64_t>)
  {
    using V = intt::intt<U, 2>;

    return detail::sqrt<T, E>(
        V(intt::direct{}, U(a.mantissa())), a.exponent()
      );
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
    using int_t = typename dpp<T, E>::int_t;

    return detail::sqrt<T, E>(
        V(intt::direct{}, D(a.mantissa())), int_t(a.exponent())
      );
  }
}

template <intt::intt_type T, typename E>
constexpr auto sqrt(dpp<T, E> const& a) noexcept
{
  using U = std::make_unsigned_t<typename T::value_type>;
  using V = intt::intt<U, 2 * T::size()>;
  using int_t = typename dpp<T, E>::int_t;

  return detail::sqrt<T, E>(
      V(a.mantissa(), intt::direct{}), int_t(a.exponent())
    );
}

}

#endif // DPP_SQRT_HPP
