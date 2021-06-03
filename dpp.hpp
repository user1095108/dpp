#ifndef DPP_HPP
# define DPP_HPP
# pragma once

#include <cmath>
#include <climits>
#include <cstdint>

#include <functional> // hash

#include <iterator>

#include <optional>

#include <ostream>

#include <string>

#include <type_traits>

namespace dpp
{

template <unsigned, unsigned>
class dpp;

using d64 = dpp<56, 8>;
using d32 = dpp<26, 6>;
using d16 = dpp<11, 5>;

using direct = struct {};
using nan = struct {};
using unpack = struct {};

namespace detail
{

template <typename U>
constexpr static auto bit_size_v(CHAR_BIT * sizeof(U));

template <typename U>
constexpr static auto is_signed_v(std::is_signed_v<U> ||
  std::is_same_v<U, __int128>);

template <typename T, T B>
constexpr T pow(unsigned e) noexcept
{
  for (T r{1}, x(B);;)
  {
    if (e % 2)
    {
      r *= x;
    }

    if (e /= 2)
    {
      x *= x;
    }
    else
    {
      return r;
    }
  }
}

constexpr int log10(__uint128_t const x, unsigned const e = 0u) noexcept
{
  return pow<__uint128_t, 10>(e) > x ? e : log10(x, e + 1);
}

template <auto a, typename B>
constexpr auto selectsign(B const b) noexcept
{
  if constexpr (is_signed_v<decltype(a)> && is_signed_v<B>)
  {
    return b >= 0 ? a : -a;
  }
  else
  {
    return a;
  }
}

// ae and be are minimal, cannot be reduced further, ae > be, maximize be.
template <typename T>
constexpr void equalize(T const am, int const ae, T& bm, int& be) noexcept
{
  if (am)
  {
    for (auto const c(detail::selectsign<T(5)>(bm)); bm && (be++ != ae);
      bm = (bm + c) / 10);

    be = ae;
  }
}

}

template <unsigned M, unsigned E>
class dpp
{
public:
  enum : unsigned { exponent_bits = E, mantissa_bits = M };

  enum : int { emin = -detail::pow<int, 2>(E - 1), emax = -(emin + 1) };

  using value_type = std::conditional_t<
    M + E <= 16,
    std::int16_t,
    std::conditional_t<
      M + E <= 32,
      std::int32_t,
      std::conditional_t<
        M + E <= 64,
        std::int64_t,
        void
      >
    >
  >;

  enum : value_type
  {
    mmin = -detail::pow<value_type, 2>(M - 1),
    mmax = -(mmin + 1)
  };

private:
  using doubled_t = std::conditional_t<
    std::is_same_v<value_type, std::int16_t>,
    std::int32_t,
    std::conditional_t<
      std::is_same_v<value_type, std::int32_t>,
      std::int64_t,
      std::conditional_t<
        std::is_same_v<value_type, std::int64_t>,
        __int128_t,
        void
      >
    >
  >;

  struct
  {
    value_type m:M;
    value_type e:E;
  } v_{};

public:
  constexpr dpp() = default;

  constexpr dpp(dpp const&) = default;
  constexpr dpp(dpp&&) = default;

  template <typename U>
  constexpr dpp(U m, int e) noexcept
    requires(std::is_integral_v<U> || std::is_same_v<U, __int128>)
  {
    constexpr auto umin(U(1) << (detail::bit_size_v<U> - 1));
    constexpr auto umax(detail::is_signed_v<U> ?  -(umin + 1) : ~U{});

    // slash m, if necessary
    if constexpr (detail::is_signed_v<U> && (detail::bit_size_v<U> > M))
    if (m < mmin)
    {
      if (m >= umin + 5)
      {
        m -= 5;
      }

      m /= 10;
      ++e;

      for (; m < mmin; m = (m - 5) / 10, ++e);
    }

    if constexpr ((detail::is_signed_v<U> && (detail::bit_size_v<U> > M)) ||
      (std::is_unsigned_v<U> && (detail::bit_size_v<U> >= M)))
    if (m > mmax)
    {
      if (m <= umax - 5)
      {
        m += 5;
      }

      m /= 10;
      ++e;

      for (; m > mmax; m = (m + 5) / 10, ++e);
    }

    // additional slashing, if necessary
    for (auto const c(detail::selectsign<U(5)>(m)); (e <= emin) && m;
      m = (m + c) / 10, ++e);

    // normalize, minimize the exponent, if m non-zero
    if (m)
    {
      value_type tm(m);

      if (m > 0)
      {
        for (; (e > emin + 1) && (tm <= mmax / 10); tm *= 10, --e);
      }
      else
      {
        for (; (e > emin + 1) && (tm >= mmin / 10); tm *= 10, --e);
      }

      if (e > emax)
      {
        *this = nan{};
      }
      else
      {
        v_.m = tm;
        v_.e = e;
      }
    }
    else
    {
      v_ = {};
    }
  }

  template <typename U>
  constexpr dpp(U const m) noexcept requires(std::is_integral_v<U>) :
    dpp(std::conditional_t<
          detail::bit_size_v<U> < detail::bit_size_v<value_type>,
          value_type,
          U
        >(m), 0
    )
  {
  }

  template <unsigned N, unsigned F>
  constexpr dpp(dpp<N, F> const o) noexcept requires((M != N) || (E != F)) :
    dpp(o.mantissa(), o.exponent())
  {
  }

  template <typename U>
  dpp(U f) noexcept requires(std::is_floating_point_v<U>)
  {
    if (std::isfinite(f))
    {
      int e{};

      // eliminate the fractional part
      for (; std::trunc(f) != f; f *= 10, --e);

      // slash f, if necessary
      for (constexpr long double
        min(std::numeric_limits<std::intmax_t>::min()),
        max(std::numeric_limits<std::intmax_t>::max());
        (f < min) || (f > max); f /= 10, ++e);

      *this = {std::intmax_t(f), e};
    }
    else
    {
      *this = nan{};
    }
  }

  //
  constexpr dpp(value_type const m, int const e, direct) noexcept :
    v_{.m = m, .e = e}
  {
  }

  constexpr dpp(nan) noexcept : v_{.m = {}, .e = emin} { }

  constexpr dpp(value_type const v, unpack) noexcept :
    v_{.m = v & (detail::pow<value_type, 2>(M) - 1), .e = v >> M}
  {
  }

  //
  constexpr dpp& operator=(dpp const&) = default;
  constexpr dpp& operator=(dpp&&) = default;

  //
  constexpr explicit operator bool() const noexcept
  {
    return isnan(*this) || v_.m;
  }

  template <typename T>
  constexpr operator T() const noexcept requires(std::is_floating_point_v<T>)
  {
    return isnan(*this) ? NAN : v_.m * std::pow(T(10), v_.e);
  }

  // this function is unsafe, take a look at to_integral() for safety
  template <typename T>
  constexpr explicit operator T() const noexcept
    requires(std::is_integral_v<T>)
  {
    if (int e(v_.e); e > 0)
    {
      return v_.m * detail::pow<T, 10>(e);
    }
    else
    {
      auto m(v_.m);

      for (; m && e++; m /= 10);

      return m;
    }
  }

  // assignment
  template <typename U>
  constexpr auto& operator+=(U const a) noexcept
  {
    return *this = *this + a;
  }

  template <typename U>
  constexpr auto& operator-=(U const a) noexcept
  {
    return *this = *this - a;
  }

  template <typename U>
  constexpr auto& operator*=(U const a) noexcept
  {
    return *this = *this * a;
  }

  template <typename U>
  constexpr auto& operator/=(U const a) noexcept
  {
    return *this = *this / a;
  }

  // increment, decrement
  constexpr auto& operator++() noexcept { return *this += 1; }
  constexpr auto& operator--() noexcept { return *this -= 1; }

  //
  constexpr int exponent() const noexcept { return v_.e; }
  constexpr auto mantissa() const noexcept { return v_.m; }

  constexpr auto packed() const noexcept
  {
    return v_.e << M | (v_.m & (detail::pow<value_type, 2>(M) - 1));
  }

  //
  template <unsigned A, unsigned B, unsigned C, unsigned D>
  friend constexpr auto operator*(dpp<A, B>, dpp<C, D>) noexcept;

  template <unsigned A, unsigned B, unsigned C, unsigned D>
  friend constexpr auto operator/(dpp<A, B>, dpp<C, D>) noexcept;
};

//arithmetic//////////////////////////////////////////////////////////////////
template <unsigned A, unsigned B>
constexpr auto operator+(dpp<A, B> const a) noexcept
{
  return a;
}

template <unsigned A, unsigned B>
constexpr auto operator-(dpp<A, B> const a) noexcept
{
  auto const m(a.mantissa());
  auto const e(a.exponent());

  // we need to do it like this, as negating the mantissa can overflow
  return dpp<A, B>::mmin == m ? dpp<A, B>(-m, e) : dpp<A, B>(-m, e, direct{});
}

//
template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator+(dpp<A, B> const a, dpp<C, D> const b) noexcept
{
  using return_t = std::conditional_t<A + B >= C + D, dpp<A, B>, dpp<C, D>>;

  if (isnan(a) || isnan(b))
  {
    return return_t{nan{}};
  }
  else
  {
    typename return_t::value_type ma(a.mantissa()), mb(b.mantissa());

    if (auto ea(a.exponent()), eb(b.exponent()); ea >= eb)
    {
      detail::equalize(ma, ea, mb, eb);

      return return_t(ma + mb, eb);
    }
    else
    {
      detail::equalize(mb, eb, ma, ea);

      return return_t(ma + mb, ea);
    }
  }
}

template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator-(dpp<A, B> const a, dpp<C, D> const b) noexcept
{
  using return_t = std::conditional_t<A + B >= C + D, dpp<A, B>, dpp<C, D>>;

  if (isnan(a) || isnan(b))
  {
    return return_t{nan{}};
  }
  else
  {
    typename return_t::value_type ma(a.mantissa()), mb(b.mantissa());

    if (auto ea(a.exponent()), eb(b.exponent()); ea >= eb)
    {
      detail::equalize(ma, ea, mb, eb);

      return return_t(ma - mb, eb);
    }
    else
    {
      detail::equalize(mb, eb, ma, ea);

      return return_t(ma - mb, ea);
    }
  }
}

template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator*(dpp<A, B> const a, dpp<C, D> const b) noexcept
{
  using return_t = std::conditional_t<A + B >= C + D, dpp<A, B>, dpp<C, D>>;

  return isnan(a) || isnan(b) ? return_t{nan{}} :
    return_t(typename return_t::doubled_t(a.v_.m) * b.v_.m, a.v_.e + b.v_.e);
}

template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator/(dpp<A, B> const a, dpp<C, D> const b) noexcept
{
  using return_t = std::conditional_t<A + B >= C + D, dpp<A, B>, dpp<C, D>>;
  using doubled_t = typename return_t::doubled_t;

  if (isnan(a) || isnan(b) || !b.v_.m) // guard against division by 0
  {
    return return_t{nan{}};
  }
  else if (doubled_t const am(a.v_.m); am)
  {
    constexpr auto rmin(doubled_t(1) << (detail::bit_size_v<doubled_t> - 1));
    constexpr auto rmax(-(rmin + 1));

    // dp is the exponent, that generates the maximal power of 10,
    // that fits into doubled_t
    // 10^dp > rmax, hence 10^(dp - 1) <= rmax
    constexpr auto dp(detail::log10(rmax) - 1);

    int e(a.v_.e - b.v_.e - dp);

    // we want an approximation to a.v_.m * (10^dp / b.v_.m)
    auto q(detail::pow<doubled_t, 10>(dp) / b.v_.m);

    // fit q * am into doubled_t
    if (auto const aam(am < 0 ? -am : am); q < 0)
    {
      for (auto const a(rmin / aam); q < a; q /= 10, ++e);
    }
    else
    {
      for (auto const a(rmax / aam); q > a; q /= 10, ++e);
    }

    return return_t(q * am, e);
  }
  else
  {
    return return_t{};
  }
}

// conversions
template <unsigned A, unsigned B, typename U>
constexpr auto operator+(dpp<A, B> const a, U const b) noexcept
  requires(std::is_arithmetic_v<U>)
{
  return a + dpp<A, B>(b);
}

template <unsigned A, unsigned B, typename U>
constexpr auto operator-(dpp<A, B> const a, U const b) noexcept
  requires(std::is_arithmetic_v<U>)
{
  return a - dpp<A, B>(b);
}

template <unsigned A, unsigned B, typename U>
constexpr auto operator*(dpp<A, B> const a, U const b) noexcept
  requires(std::is_arithmetic_v<U>)
{
  return a * dpp<A, B>(b);
}

template <unsigned A, unsigned B, typename U>
constexpr auto operator/(dpp<A, B> const a, U const b) noexcept
  requires(std::is_arithmetic_v<U>)
{
  return a / dpp<A, B>(b);
}

// conversions
template <unsigned A, unsigned B, typename U>
constexpr auto operator+(U const a, dpp<A, B> const b) noexcept
  requires(std::is_arithmetic_v<U>)
{
  return dpp<A, B>(a) + b;
}

template <unsigned A, unsigned B, typename U>
constexpr auto operator-(U const a, dpp<A, B> const b) noexcept
  requires(std::is_arithmetic_v<U>)
{
  return dpp<A, B>(a) - b;
}

template <unsigned A, unsigned B, typename U>
constexpr auto operator*(U const a, dpp<A, B> const b) noexcept
  requires(std::is_arithmetic_v<U>)
{
  return dpp<A, B>(a) * b;
}

template <unsigned A, unsigned B, typename U>
constexpr auto operator/(U const a, dpp<A, B> const b) noexcept
  requires(std::is_arithmetic_v<U>)
{
  return dpp<A, B>(a) / b;
}

//increment, decrement////////////////////////////////////////////////////////
template <unsigned A, unsigned B>
constexpr auto operator++(dpp<A, B> const a, int) noexcept
{
  return a + 1;
}

template <unsigned A, unsigned B>
constexpr auto operator--(dpp<A, B> const a, int) noexcept
{
  return a - 1;
}

//comparison//////////////////////////////////////////////////////////////////
template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator==(dpp<A, B> const a, dpp<C, D> const b) noexcept
{
  return !isnan(a) && !isnan(b) &&
    (a.exponent() == b.exponent()) && (a.mantissa() == b.mantissa());
}

template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator!=(dpp<A, B> const a, dpp<C, D> const b) noexcept
{
  return !(a == b);
}

//
template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator<(dpp<A, B> const a, dpp<C, D> const b) noexcept
{
  using return_t = std::conditional_t<A + B >= C + D, dpp<A, B>, dpp<C, D>>;

  if (isnan(a) || isnan(b))
  {
    return false;
  }
  else
  {
    typename return_t::value_type ma(a.mantissa()), mb(b.mantissa());

    if (auto ea(a.exponent()), eb(b.exponent()); ea >= eb)
    {
      detail::equalize(ma, ea, mb, eb);
    }
    else
    {
      detail::equalize(mb, eb, ma, ea);
    }

    return ma < mb;
  }
}

template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator>(dpp<A, B> const a, dpp<C, D> const b) noexcept
{
  return b < a;
}

template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator<=(dpp<A, B> const a, dpp<C, D> const b) noexcept
{
  return !(b < a);
}

template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator>=(dpp<A, B> const a, dpp<C, D> const b) noexcept
{
  return !(a < b);
}

template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator<=>(dpp<A, B> const a, dpp<C, D> const b) noexcept
{
  return (a > b) - (a < b);
}

// conversions
template <unsigned A, unsigned B, typename U>
constexpr auto operator==(dpp<A, B> const a, U const b) noexcept
  requires(std::is_arithmetic_v<U>)
{
  return a == dpp<A, B>(b);
}

template <unsigned A, unsigned B, typename U>
constexpr auto operator!=(dpp<A, B> const a, U const b) noexcept
  requires(std::is_arithmetic_v<U>)
{
  return a != dpp<A, B>(b);
}

template <unsigned A, unsigned B, typename U>
constexpr auto operator<(dpp<A, B> const a, U const b) noexcept
  requires(std::is_arithmetic_v<U>)
{
  return a < dpp<A, B>(b);
}

template <unsigned A, unsigned B, typename U>
constexpr auto operator>(dpp<A, B> const a, U const b) noexcept
  requires(std::is_arithmetic_v<U>)
{
  return a > dpp<A, B>(b);
}

template <unsigned A, unsigned B, typename U>
constexpr auto operator<=(dpp<A, B> const a, U const b) noexcept
  requires(std::is_arithmetic_v<U>)
{
  return a <= dpp<A, B>(b);
}

template <unsigned A, unsigned B, typename U>
constexpr auto operator>=(dpp<A, B> const a, U const b) noexcept
  requires(std::is_arithmetic_v<U>)
{
  return a >= dpp<A, B>(b);
}

// conversions
template <unsigned A, unsigned B, typename U>
constexpr auto operator==(U const a, dpp<A, B> const b) noexcept
  requires(std::is_arithmetic_v<U>)
{
  return dpp<A, B>(a) == b;
}

template <unsigned A, unsigned B, typename U>
constexpr auto operator!=(U const a, dpp<A, B> const b) noexcept
  requires(std::is_arithmetic_v<U>)
{
  return dpp<A, B>(a) != b;
}

template <unsigned A, unsigned B, typename U>
constexpr auto operator<(U const a, dpp<A, B> const b) noexcept
  requires(std::is_arithmetic_v<U>)
{
  return dpp<A, B>(a) < b;
}

template <unsigned A, unsigned B, typename U>
constexpr auto operator>(U const a, dpp<A, B> const b) noexcept
  requires(std::is_arithmetic_v<U>)
{
  return dpp<A, B>(a) > b;
}

template <unsigned A, unsigned B, typename U>
constexpr auto operator<=(U const a, dpp<A, B> const b) noexcept
  requires(std::is_arithmetic_v<U>)
{
  return dpp<A, B>(a) <= b;
}

template <unsigned A, unsigned B, typename U>
constexpr auto operator>=(U const a, dpp<A, B> const b) noexcept
  requires(std::is_arithmetic_v<U>)
{
  return dpp<A, B>(a) >= b;
}

//misc////////////////////////////////////////////////////////////////////////
template <unsigned M, unsigned E>
constexpr auto isnan(dpp<M, E> const a) noexcept
{
  return dpp<M, E>::emin == a.exponent();
}

//
template <unsigned M, unsigned E>
constexpr auto trunc(dpp<M, E> const a) noexcept
{
  if (auto e(a.exponent()); !isnan(a) && (e < 0))
  {
    auto m(a.mantissa());

    for (; m && e++; m /= 10);

    return dpp<M, E>(m, 0);
  }
  else
  {
    return a;
  }
}

template <unsigned M, unsigned E>
constexpr auto ceil(dpp<M, E> const a) noexcept
{
  auto const t(trunc(a));

  return dpp<M, E>(t.mantissa() + (t < a), t.exponent());
}

template <unsigned M, unsigned E>
constexpr auto floor(dpp<M, E> const a) noexcept
{
  auto const t(trunc(a));

  return dpp<M, E>(t.mantissa() - (t > a), t.exponent());
}

template <unsigned M, unsigned E>
constexpr auto round(dpp<M, E> const a) noexcept
{
  constexpr dpp<M, E> c(5, -1);

  return !isnan(a) && (a.exponent() < 0) ?
    trunc(a.mantissa() > 0 ? a + c : a - c) :
    a;
}

template <unsigned M, unsigned E>
constexpr auto abs(dpp<M, E> const a) noexcept
{
  return a.mantissa() < 0 ? -a : a;
}

template <unsigned M, unsigned E>
constexpr auto frac(dpp<M, E> const a) noexcept
{
  return a - trunc(a);
}

template <unsigned M, unsigned E>
constexpr auto sign(dpp<M, E> const a) noexcept
{
  auto const m(a.mantissa());

  return (m > 0) - (m < 0);
}

//////////////////////////////////////////////////////////////////////////////
template <typename T, typename It>
constexpr T to_decimal(It i, It const end) noexcept
{
  if (i == end)
  {
    return nan{};
  }
  else
  {
    bool positive{true};

    switch (*i)
    {
      case '+':
        i = std::next(i);
        break;

      case '-':
        positive = false;
        i = std::next(i);
        break;

      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
      case '.':
        break;

      default:
        return nan{};
    }

    typename T::value_type r{};

    constexpr auto rmax(std::numeric_limits<std::intmax_t>::max());
    constexpr auto rmin(std::numeric_limits<std::intmax_t>::min());

    for (; i != end; i = std::next(i))
    {
      switch (*i)
      {
        case '.':
          i = std::next(i);
          break;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          if (positive)
          {
            if (r <= rmax / 10)
            {
              r *= 10;

              if (auto const d(*i - '0'); r <= rmax - d)
              {
                r += d;

                continue;
              }
            }
          }
          else
          {
            if (r >= rmin / 10)
            {
              r *= 10;

              if (auto const d(*i - '0'); r >= rmin + d)
              {
                r -= d;

                continue;
              }
            }
          }

          break;

        case '\0':
        {
          return {r, 0};
        }

        default:
          return nan{};
      }

      break;
    }

    int e{};

    for (; i != end; i = std::next(i))
    {
      switch (*i)
      {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          if (positive)
          {
            if ((e > T::emin + 1) && (r <= rmax / 10))
            {
              r *= 10;

              if (auto const d(*i - '0'); r <= rmax - d)
              {
                r += d;
                --e;

                continue;
              }
            }
          }
          else
          {
            if ((e > T::emin + 1) && (r >= rmin / 10))
            {
              r *= 10;

              if (auto const d(*i - '0'); r >= rmin + d)
              {
                r -= d;
                --e;

                continue;
              }
            }
          }

          break;

        case '\0':
          break;

        default:
          return nan{};
      }

      break;
    }

    return {r, e};
  }
}

template <typename T, typename S>
constexpr auto to_decimal(S const& s) noexcept ->
  decltype(std::cbegin(s), std::cend(s), T())
{
  return to_decimal<T>(std::cbegin(s), std::cend(s));
}

//////////////////////////////////////////////////////////////////////////////
template <typename T = std::intmax_t, unsigned M, unsigned E>
constexpr std::optional<T> to_integral(dpp<M, E> const p) noexcept
{
  if (isnan(p))
  {
    return {};
  }
  else
  {
    T m(p.mantissa());

    if (auto e(p.exponent()); e <= 0)
    {
      for (; m && e++; m /= 10);

      return m;
    }
    else
    {
      do
      {
        if ((m >= std::numeric_limits<T>::min() / 10) &&
          (m <= std::numeric_limits<T>::max() / 10))
        {
          m *= 10;
        }
        else
        {
          return {};
        }
      }
      while (--e);

      return m;
    }
  }
}

template <unsigned M, unsigned E>
std::string to_string(dpp<M, E> p)
{
  if (isnan(p))
  {
    return {"nan", 3};
  }
  else
  {
    std::string r;

    if (auto const t(trunc(p)); t)
    {
      p -= t;

      auto m(t.mantissa());
      auto e(t.exponent());

      for (; !(m % 10); m /= 10, ++e);

      r.append(std::to_string(m)).append(e, '0');
    }
    else
    {
      p.mantissa() < 0 ? r.append("-0", 2) : r.append(1, '0');
    }

    auto m(p.mantissa());

    if (auto e(-p.exponent()); (e > 0) && m)
    {
      for (; !(m % 10); m /= 10, --e);

      auto const tmp(std::to_string(std::abs(m)));

      if (auto const s(tmp.size()); std::size_t(e) >= s)
      {
        r.append(1, '.').append(e - s, '0').append(tmp);
      }
      else
      {
        return {"nan", 3};
      }
    }

    return r;
  }
}

template <unsigned M, unsigned E>
inline auto& operator<<(std::ostream& os, dpp<M, E> const p)
{
  return os << to_string(p);
}

//////////////////////////////////////////////////////////////////////////////
namespace literals
{

template <char ...c>
constexpr auto operator "" _d64() noexcept
{
  return to_decimal<d64>((char const[sizeof...(c)]){c...});
}

template <char ...c>
constexpr auto operator "" _d32() noexcept
{
  return to_decimal<d32>((char const[sizeof...(c)]){c...});
}

template <char ...c>
constexpr auto operator "" _d16() noexcept
{
  return to_decimal<d16>((char const[sizeof...(c)]){c...});
}

}

}

namespace std
{

template <unsigned M, unsigned E>
struct hash<dpp::dpp<M, E>>
{
  constexpr auto operator()(dpp::dpp<M, E> const a) const noexcept
  {
    return std::hash<decltype(a.packed())>()(a.packed());
  }
};

}

#endif // DPP_HPP
