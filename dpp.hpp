#ifndef DPP_HPP
# define DPP_HPP
# pragma once

#include <cmath>

#include <cstdint>

#include <functional>

#include <iterator>

#include <optional>

#include <ostream>

#include <string_view>

#include <type_traits>

#include <utility>

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

template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator+(dpp<A, B>, dpp<C, D>) noexcept;

template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator-(dpp<A, B>, dpp<C, D>) noexcept;

template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator*(dpp<A, B>, dpp<C, D>) noexcept;

template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator/(dpp<A, B>, dpp<C, D>) noexcept;

template <unsigned M, unsigned E>
constexpr bool isnan(dpp<M, E>) noexcept;

template <typename T, typename S>
constexpr auto to_decimal(S const& s) noexcept ->
  decltype(std::cbegin(s), std::cend(s), T());

template <typename T, unsigned M, unsigned E>
constexpr T to_float(dpp<M, E>) noexcept;

namespace
{

template <typename U>
constexpr auto bit_size() noexcept
{
  return 8 * sizeof(U);
}

template <unsigned B, typename T>
constexpr T pow(unsigned e) noexcept
{
  if (e)
  {
    T x(B), y(1);

    while (1 != e)
    {
      if (e % 2)
      {
        //--e;
        y *= x;
      }

      x *= x;
      e /= 2;
    }

    return x * y;
  }
  else
  {
    return T(1);
  }
}

constexpr int log10(__uint128_t const x, unsigned e = 0u) noexcept
{
  return pow<10, __uint128_t>(e) > x ? e : log10(x, e + 1);
}

template <unsigned E, typename T>
constexpr bool equalize(T& am, int& ae, T& bm, int& be) noexcept
{
  constexpr auto emin(-pow<2, int>(E - 1));
  constexpr auto emax(-(emin + 1));

  // reserve one bit in case of overflow
  constexpr auto rmin(-pow<2, T>(bit_size<T>() - 2));
  constexpr auto rmax(-(rmin + 1));

  if (am > 0)
  {
    while ((ae != be) && (am <= rmax / 10) && (ae > emin + 1))
    {
      --ae;
      am *= 10;
    }
  }
  else if (am < 0)
  {
    while ((ae != be) && (am >= rmin / 10) && (ae > emin + 1))
    {
      --ae;
      am *= 10;
    }
  }
  else
  {
    ae = be;

    return false;
  }

  if (ae != be)
  {
    if (bm > 0)
    {
      while (ae != be)
      {
        if (be <= emax - 1)
        {
          ++be;

          if (bm <= rmax - 5)
          {
            bm += 5;
          }

          bm /= 10;
        }
        else
        {
          return true;
        }
      }
    }
    else if (bm < 0)
    {
      while (ae != be)
      {
        if (be <= emax - 1)
        {
          ++be;

          if (bm >= rmin + 5)
          {
            bm -= 5;
          }

          bm /= 10;
        }
        else
        {
          return true;
        }
      }
    }
/*
    else
    {
      be = ae;
    }
*/
  }

  return false;
}

}

template <unsigned M, unsigned E>
class dpp
{
public:
  enum : unsigned { exponent_bits = E, mantissa_bits = M };

  enum : int { emin = -pow<2, int>(E - 1), emax = -(emin + 1) };

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

  enum : value_type { mmin = -pow<2, value_type>(M - 1), mmax = -(mmin + 1) };

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

  constexpr void normalize() noexcept
  {
    if (v_.m)
    {
      while (!(v_.m % 10))
      {
        if (v_.e <= emax - 1)
        {
          ++v_.e;
          v_.m /= 10;
        }
        else
        {
          *this = dpp{nan{}};

          break;
        }
      }
    }
    else
    {
      v_.e = 0;
    }
  }

public:
  constexpr dpp() = default;

  constexpr dpp(dpp const&) = default;
  constexpr dpp(dpp&&) = default;

  template <typename U,
    std::enable_if_t<
      std::is_integral_v<U> ||
      std::is_same_v<U, __int128>,
      int
    > = 0
  >
  constexpr dpp(U m, int e) noexcept
  {
    if constexpr (std::is_same_v<U, bool>)
    {
      v_.m = m;
      (void)e;
    }
    else
    {
      if (e > emax)
      {
        *this = dpp{nan{}};

        return;
      }
      else
      {
        // watch the nan
        while ((e <= emin) && m)
        {
          ++e;
          m /= 10;
        }
      }

      constexpr auto umin(U(1) << (bit_size<U>() - 1));
      constexpr auto umax(std::is_signed_v<U> || std::is_same_v<U, __int128> ?
        -(umin + 1) : ~U{});

      if constexpr (std::is_signed_v<U> || std::is_same_v<U, __int128>)
      while (m < mmin)
      {
        if (e <= emax - 1)
        {
          ++e;

          if (m >= umin + 5)
          {
            m -= 5;
          }

          m /= 10;
        }
        else
        {
          *this = dpp{nan{}};

          return;
        }
      }

      while (m > mmax)
      {
        if (e <= emax - 1)
        {
          ++e;

          if (m <= umax - 5)
          {
            m += 5;
          }

          m /= 10;
        }
        else
        {
          *this = dpp{nan{}};

          return;
        }
      }

      v_.m = m;
      v_.e = e;

      normalize();
    }
  }

  template <typename U,
    std::enable_if_t<std::is_integral_v<U>, int> = 0
  >
  constexpr dpp(U const m) noexcept :
    dpp(m, 0)
  {
  }

  template <unsigned N, unsigned F,
    std::enable_if_t<(M < N) || (E < F), int> = 0
  >
  constexpr dpp(dpp<N, F> const o) noexcept :
    dpp(o.mantissa(), o.exponent())
  {
  }

  template <unsigned N, unsigned F,
    std::enable_if_t<(M >= N) && (E >= F), int> = 0
  >
  constexpr dpp(dpp<N, F> const o) noexcept :
    dpp(o.mantissa(), o.exponent(), direct{})
  {
  }

  template <typename U,
    typename = std::enable_if_t<std::is_floating_point_v<U>>
  >
  dpp(U f) noexcept
  {
    if (std::isfinite(f))
    {
      int e{};

      // eliminate the fractional part
      for (; std::trunc(f) != f; f *= 10, --e);

      // slash f, if necessary
      for (constexpr long double
        max(std::numeric_limits<std::intmax_t>::max()),
        min(std::numeric_limits<std::intmax_t>::min());
        (f > max) || (f < min); f /= 10, ++e);

      *this = dpp{std::intmax_t(f), e};
    }
    else
    {
      *this = dpp{nan{}};
    }
  }

  //
  template <typename U, std::size_t N,
    std::enable_if_t<std::is_same_v<char, std::remove_cv_t<U>>, int> = 0
  >
  constexpr dpp(U(&s)[N]) noexcept
  {
    *this = s;
  }

  constexpr dpp(std::string_view const& s) noexcept
  {
    *this = s;
  }

  constexpr dpp(value_type const m, int const e, direct&&) noexcept :
    v_{.m = m, .e = value_type(e)}
  {
  }

  constexpr dpp(nan&&) noexcept :
    v_{.m = {}, .e = emin}
  {
  }

  constexpr dpp(value_type const v, unpack&&) noexcept :
    v_{.m = v & (pow<2, value_type>(M) - 1), .e = v >> M}
  {
  }

  //
  constexpr dpp& operator=(dpp const&) = default;
  constexpr dpp& operator=(dpp&&) = default;

  template <typename U, std::size_t N,
    std::enable_if_t<std::is_same_v<char, std::remove_cv_t<U>>, int> = 0
  >
  constexpr auto& operator=(U(&s)[N]) noexcept
  {
    return *this = to_decimal<dpp>(s);
  }

  constexpr auto& operator=(std::string_view const& s) noexcept
  {
    return *this = to_decimal<dpp>(s);
  }

  //
  constexpr explicit operator bool() const noexcept
  {
    return isnan(*this) || v_.m;
  }

  template <typename T,
    std::enable_if_t<std::is_floating_point_v<T>, int> = 0
  >
  constexpr operator T() const noexcept
  {
    return to_float<T>(*this);
  }

  // this function is unsafe, take a look at to_integral() for safety
  template <typename T,
     std::enable_if_t<
      !std::is_same_v<std::remove_cv_t<T>, bool> &&
      std::is_integral_v<T>,
      int
    > = 0
  >
  constexpr explicit operator T() const noexcept
  {
    if (int e(v_.e); e < 0)
    {
      auto m(v_.m);

      for (; m && e++; m /= 10);

      return m;
    }
    else
    {
      return v_.m * pow<10, T>(e);
    }
  }

  // assignment
  template <typename U>
  constexpr auto& operator=(U&& a) noexcept
  {
    return *this = dpp(std::forward<U>(a));
  }

  //
  template <typename U>
  constexpr auto& operator+=(U&& a) noexcept
  {
    return *this = *this + dpp(std::forward<U>(a));
  }

  template <typename U>
  constexpr auto& operator-=(U&& a) noexcept
  {
    return *this = *this - dpp(std::forward<U>(a));
  }

  template <typename U>
  constexpr auto& operator*=(U&& a) noexcept
  {
    return *this = *this * dpp(std::forward<U>(a));
  }

  template <typename U>
  constexpr auto& operator/=(U&& a) noexcept
  {
    return *this = *this / dpp(std::forward<U>(a));
  }

  // increment, decrement
  constexpr auto& operator++() noexcept { return *this = *this + dpp(1); }
  constexpr auto& operator--() noexcept { return *this = *this - dpp(1); }

  //
  constexpr int exponent() const noexcept
  {
    return v_.e;
  }

  constexpr auto mantissa() const noexcept
  {
    return v_.m;
  }

  constexpr auto packed() const noexcept
  {
    return v_.e << M | (v_.m & (pow<2, value_type>(M) - 1));
  }

  //
  template <unsigned A, unsigned B, unsigned C, unsigned D>
  friend constexpr auto operator+(dpp<A, B>, dpp<C, D>) noexcept;

  template <unsigned A, unsigned B, unsigned C, unsigned D>
  friend constexpr auto operator-(dpp<A, B>, dpp<C, D>) noexcept;

  template <unsigned A, unsigned B, unsigned C, unsigned D>
  friend constexpr auto operator*(dpp<A, B>, dpp<C, D>) noexcept;

  template <unsigned A, unsigned B, unsigned C, unsigned D>
  friend constexpr auto operator/(dpp<A, B>, dpp<C, D>) noexcept;
};

//increment, decrement////////////////////////////////////////////////////////
template <unsigned A, unsigned B>
constexpr auto operator++(dpp<A, B> const a, int) noexcept
{
  return dpp<A, B>(a + dpp<A, B>(1));
}

template <unsigned A, unsigned B>
constexpr auto operator--(dpp<A, B> const a, int) noexcept
{
  return dpp<A, B>(a - dpp<A, B>(1));
}

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
  using return_t = dpp<(A > C ? A : C), (A > C ? B : D)>;

  if (isnan(a) || isnan(b))
  {
    return return_t{nan{}};
  }
  else
  {
    typename return_t::value_type ma(a.v_.m), mb(b.v_.m);
    int ea(a.v_.e), eb(b.v_.e);

    if (((ea > eb) && equalize<return_t::exponent_bits>(ma, ea, mb, eb)) ||
      ((eb > ea) && equalize<return_t::exponent_bits>(mb, eb, ma, ea)))
    {
      return return_t{nan{}};
    }

    // there can be no overflow
    return return_t(ma + mb, ea);
  }
}

template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator-(dpp<A, B> const a, dpp<C, D> const b) noexcept
{
  using return_t = dpp<(A > C ? A : C), (A > C ? B : D)>;

  if (isnan(a) || isnan(b))
  {
    return return_t{nan{}};
  }
  else
  {
    typename return_t::value_type ma(a.v_.m), mb(b.v_.m);
    int ea(a.v_.e), eb(b.v_.e);

    if (((ea > eb) && equalize<return_t::exponent_bits>(ma, ea, mb, eb)) ||
      ((eb > ea) && equalize<return_t::exponent_bits>(mb, eb, ma, ea)))
    {
      return return_t{nan{}};
    }

    // there can be no overflow
    return return_t(ma - mb, ea);
  }
}

template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator*(dpp<A, B> const a, dpp<C, D> const b) noexcept
{
  using return_t = dpp<(A > C ? A : C), (A > C ? B : D)>;

  return isnan(a) || isnan(b) ? return_t{nan{}} :
    return_t(typename return_t::doubled_t(a.v_.m) * b.v_.m, a.v_.e + b.v_.e);
}

template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator/(dpp<A, B> const a, dpp<C, D> const b) noexcept
{
  using return_t = dpp<(A > C ? A : C), (A > C ? B : D)>;

  if (isnan(a) || isnan(b) || !b.v_.m) // guard against division by 0
  {
    return return_t{nan{}};
  }
  else if (auto am(a.v_.m); am) // guard against division by 0
  {
    constexpr auto rmin(typename return_t::doubled_t(1) <<
      (bit_size<typename return_t::doubled_t>() - 1));
    constexpr auto rmax(-(rmin + 1));

    // dp is the exponent, that generates the maximal power of 10,
    // that fits into doubled_t
    constexpr auto dp(log10(rmax) - 1);

    int e(a.v_.e - b.v_.e - dp);

    // we want an approximation to a.v_.m * (10^dp / b.v_.m)
    auto const q(pow<10, typename return_t::doubled_t>(dp) / b.v_.m);

    // negating both am and r does not change the quotient
    auto r(am < 0 ? am = -am, -q : q);

    // fit r * am into doubled_t, avoid one divide, there are no sign changes
    if (r > 0)
    {
      while (r > rmax / am)
      {
        r /= 10;
        ++e;
      }
    }
    else if (r < 0)
    {
      while (r < rmin / am)
      {
        r /= 10;
        ++e;
      }
    }

    return return_t(r * am, e);
  }
  else
  {
    return return_t{};
  }
}

// conversions
template <unsigned A, unsigned B, typename T>
constexpr auto operator+(dpp<A, B> const a, T const b) noexcept
{
  return a + dpp<A, B>(b);
}

template <unsigned A, unsigned B, typename T>
constexpr auto operator-(dpp<A, B> const a, T const b) noexcept
{
  return a - dpp<A, B>(b);
}

template <unsigned A, unsigned B, typename T>
constexpr auto operator*(dpp<A, B> const a, T const b) noexcept
{
  return a * dpp<A, B>(b);
}

template <unsigned A, unsigned B, typename T>
constexpr auto operator/(dpp<A, B> const a, T const b) noexcept
{
  return a / dpp<A, B>(b);
}

// conversions
template <unsigned A, unsigned B, typename T>
constexpr auto operator+(T const a, dpp<A, B> const b) noexcept
{
  return dpp<A, B>(a) + b;
}

template <unsigned A, unsigned B, typename T>
constexpr auto operator-(T const a, dpp<A, B> const b) noexcept
{
  return dpp<A, B>(a) - b;
}

template <unsigned A, unsigned B, typename T>
constexpr auto operator*(T const a, dpp<A, B> const b) noexcept
{
  return dpp<A, B>(a) * b;
}

template <unsigned A, unsigned B, typename T>
constexpr auto operator/(T const a, dpp<A, B> const b) noexcept
{
  return dpp<A, B>(a) / b;
}

//comparison//////////////////////////////////////////////////////////////////
template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator==(dpp<A, B> const a, dpp<C, D> const b) noexcept
{
  return isnan(a) || isnan(b) ? false :
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
  using return_t = dpp<(A > C ? A : C), (A > C ? B : D)>;

  if (isnan(a) || isnan(b))
  {
    return false;
  }
  else
  {
    typename return_t::value_type ma(a.mantissa()), mb(b.mantissa());
    auto ea(a.exponent()), eb(b.exponent());

    if (((ea > eb) && equalize<return_t::exponent_bits>(ma, ea, mb, eb)) ||
      ((eb > ea) && equalize<return_t::exponent_bits>(mb, eb, ma, ea)))
    {
      return false;
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

#if __cplusplus > 201703L
template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator<=>(dpp<A, B> const a, dpp<C, D> const b) noexcept
{
  return (a > b) - (a < b);
}
#endif

// conversions
template <unsigned A, unsigned B, typename T>
constexpr auto operator==(dpp<A, B> const a, T const b) noexcept
{
  return a == dpp<A, B>(b);
}

template <unsigned A, unsigned B, typename T>
constexpr auto operator!=(dpp<A, B> const a, T const b) noexcept
{
  return a != dpp<A, B>(b);
}

template <unsigned A, unsigned B, typename T>
constexpr auto operator<(dpp<A, B> const a, T const b) noexcept
{
  return a < dpp<A, B>(b);
}

template <unsigned A, unsigned B, typename T>
constexpr auto operator>(dpp<A, B> const a, T const b) noexcept
{
  return a > dpp<A, B>(b);
}

template <unsigned A, unsigned B, typename T>
constexpr auto operator<=(dpp<A, B> const a, T const b) noexcept
{
  return a <= dpp<A, B>(b);
}

template <unsigned A, unsigned B, typename T>
constexpr auto operator>=(dpp<A, B> const a, T const b) noexcept
{
  return a >= dpp<A, B>(b);
}

// conversions
template <unsigned A, unsigned B, typename T>
constexpr auto operator==(T const a, dpp<A, B> const b) noexcept
{
  return dpp<A, B>(a) == b;
}

template <unsigned A, unsigned B, typename T>
constexpr auto operator!=(T const a, dpp<A, B> const b) noexcept
{
  return dpp<A, B>(a) != b;
}

template <unsigned A, unsigned B, typename T>
constexpr auto operator<(T const a, dpp<A, B> const b) noexcept
{
  return dpp<A, B>(a) < b;
}

template <unsigned A, unsigned B, typename T>
constexpr auto operator>(T const a, dpp<A, B> const b) noexcept
{
  return dpp<A, B>(a) > b;
}

template <unsigned A, unsigned B, typename T>
constexpr auto operator<=(T const a, dpp<A, B> const b) noexcept
{
  return dpp<A, B>(a) <= b;
}

template <unsigned A, unsigned B, typename T>
constexpr auto operator>=(T const a, dpp<A, B> const b) noexcept
{
  return dpp<A, B>(a) >= b;
}

//misc////////////////////////////////////////////////////////////////////////
template <unsigned M, unsigned E>
constexpr bool isnan(dpp<M, E> const a) noexcept
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

    return dpp<M, E>(m, 0, direct{});
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

  return t + dpp<M, E>(t < a);
}

template <unsigned M, unsigned E>
constexpr auto floor(dpp<M, E> const a) noexcept
{
  auto const t(trunc(a));

  return t - dpp<M, E>(t > a);
}

template <unsigned M, unsigned E>
constexpr auto round(dpp<M, E> const a) noexcept
{
  constexpr dpp<M, E> c(5, -1);

  return !isnan(a) && (a.exponent() < 0) ?
    trunc(a.mantissa() >= 0 ? a + c : a - c) :
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
  constexpr auto m(a.mantissa());

  return (m > 0) - (m < 0);
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
    auto m(p.mantissa());

    if (auto e(p.exponent()); e <= 0)
    {
      for (; m && e++; m /= 10);

      return m;
    }
    else
    {
      if (m > 0)
      {
        for (; e--;)
        {
          if (m <= std::numeric_limits<T>::max() / 10)
          {
            m *= 10;
          }
          else
          {
            return {};
          }
        }
      }
      else
      {
        for (; e--;)
        {
          if (m >= std::numeric_limits<T>::min() / 10)
          {
            m *= 10;
          }
          else
          {
            return {};
          }
        }
      }

      return m;
    }
  }
}

template <typename T, typename It>
constexpr T to_decimal(It i, It const end) noexcept
{
  if (i == end)
  {
    return {nan{}};
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
        return {nan{}};
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
          return {nan{}};
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
          return {nan{}};
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

template <typename T, unsigned M, unsigned E>
constexpr T to_float(dpp<M, E> const a) noexcept
{
  return isnan(a) ? NAN : a.mantissa() * std::pow(T(10), a.exponent());
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

    auto m(p.mantissa());

    if (auto const t(trunc(p)); t)
    {
      p -= t;
      m = p.mantissa();

      r.append(std::to_string(t.mantissa())).append(t.exponent(), '0');
    }
    else
    {
      m < 0 ? r.append("-0", 2) : r.append(1, '0');
    }

    if (auto const e(-p.exponent()); (e > 0) && m)
    {
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

constexpr auto operator "" _d64(char const* const s,
  std::size_t const N) noexcept
{
  return to_decimal<d64>(std::string_view(s, N));
}

constexpr auto operator "" _d32(char const* const s,
  std::size_t const N) noexcept
{
  return to_decimal<d32>(std::string_view(s, N));
}

constexpr auto operator "" _d16(char const* const s,
  std::size_t const N) noexcept
{
  return to_decimal<d16>(std::string_view(s, N));
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
