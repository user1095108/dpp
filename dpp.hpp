#ifndef DPP_HPP
# define DPP_HPP
# pragma once

#include <cmath>

#include <cstdint>

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

template <typename T>
struct decimal_places : std::conditional_t<
  std::is_same_v<T, __int128_t>,
  std::integral_constant<int, 38>,
  std::conditional_t<
    std::is_same_v<T, std::int64_t>,
    std::integral_constant<int, 18>,
    std::conditional_t<
      std::is_same_v<T, std::int32_t>,
      std::integral_constant<int, 9>,
      std::conditional_t<
        std::is_same_v<T, std::int16_t>,
        std::integral_constant<int, 4>,
        void
      >
    >
  >
>
{
};

template <typename U>
constexpr auto bit_size() noexcept
{
  return 8 * sizeof(U);
}

template <int B, typename T>
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
  enum : unsigned
  {
    exponent_bits = E,
    mantissa_bits = M
  };

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

private:
  struct
  {
    value_type m:M;
    value_type e:E;
  } v_{};

  constexpr void normalize() noexcept
  {
    constexpr auto emax(pow<2, int>(E - 1) - 1);

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
  constexpr dpp() noexcept = default;

  constexpr dpp(dpp const&) = default;
  constexpr dpp(dpp&&) = default;

  template <typename U,
    std::enable_if_t<
      std::is_integral_v<U> ||
      std::is_same_v<U, __int128>,
      int
    > = 0
  >
  constexpr dpp(U m, int const e) noexcept
  {
    if constexpr (std::is_same_v<U, bool>)
    {
      v_.m = m;
      (void)e;
    }
    else
    {
      constexpr auto emin(-pow<2, int>(E - 1));
      constexpr auto emax(-(emin + 1));

      // watch the nan
      if ((e > emin) && (e <= emax))
      {
        v_.e = e;
      }
      else
      {
        *this = dpp{nan{}};

        return;
      }

      constexpr auto mmin(-pow<2, value_type>(M - 1));
      constexpr auto mmax(-(mmin + 1));

      constexpr auto umin(U(1) << (bit_size<U>() - 1));
      constexpr auto umax(std::is_signed_v<U> || std::is_same_v<U, __int128> ?
        -(umin + 1) : ~U{});

      if constexpr (std::is_signed_v<U> || std::is_same_v<U, __int128>)
      while (m < mmin)
      {
        if (v_.e <= emax - 1)
        {
          ++v_.e;

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
        if (v_.e <= emax - 1)
        {
          ++v_.e;

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
  constexpr dpp(dpp<N, F> const& o) noexcept :
    dpp(o.mantissa(), o.exponent())
  {
  }

  template <unsigned N, unsigned F,
    std::enable_if_t<(M >= N) && (E >= F), int> = 0
  >
  constexpr dpp(dpp<N, F> const& o) noexcept :
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
      for (constexpr long double max(std::numeric_limits<std::intmax_t>::max()),
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

  struct direct{};

  constexpr dpp(value_type const m, int const e, direct&&) noexcept :
    v_{.m = m, .e = e}
  {
  }

  struct nan{};

  constexpr dpp(nan&&) noexcept :
    v_{.m = {}, .e = -pow<2, int>(E - 1)}
  {
  }

  struct unpack{};

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

  constexpr explicit operator std::intmax_t() const noexcept
  {
    return v_.e < 0 ? v_.m / pow<10>(-v_.e) : v_.m * pow<10>(v_.e);
  }

  constexpr explicit operator float() const noexcept
  {
    return to_float<float>(*this);
  }

  constexpr explicit operator double() const noexcept
  {
    return to_float<double>(*this);
  }

  constexpr explicit operator long double() const noexcept
  {
    return to_float<long double>(*this);
  }

  //
  constexpr auto& operator+() const noexcept
  {
    return *this;
  }

  constexpr auto operator-() const noexcept
  {
    auto const m(v_.m);

    // we need to do it like this, as negating the mantissa can overflow
    return -pow<2, value_type>(M - 1) == m ? dpp(-m, v_.e) :
      dpp(-m, v_.e, direct{});
  }

  //
  template <unsigned N, unsigned F>
  constexpr auto& operator+=(dpp<N, F> const o) noexcept
  {
    return *this = *this + o;
  }

  template <unsigned N, unsigned F>
  constexpr auto& operator-=(dpp<N, F> const o) noexcept
  {
    return *this = *this - o;
  }

  template <unsigned N, unsigned F>
  constexpr auto& operator*=(dpp<N, F> const o) noexcept
  {
    return *this = *this * o;
  }

  template <unsigned N, unsigned F>
  constexpr auto& operator/=(dpp<N, F> const o) noexcept
  {
    return *this = *this / o;
  }

  //
  constexpr int exponent() const noexcept
  {
    return v_.e;
  }

  constexpr value_type mantissa() const noexcept
  {
    return v_.m;
  }

  constexpr auto packed() const noexcept
  {
    return (v_.e << M) | v_.m;
  }

  template <unsigned A, unsigned B, unsigned C, unsigned D>
  friend constexpr auto operator+(dpp<A, B>, dpp<C, D>) noexcept;

  template <unsigned A, unsigned B, unsigned C, unsigned D>
  friend constexpr auto operator-(dpp<A, B>, dpp<C, D>) noexcept;

  template <unsigned A, unsigned B, unsigned C, unsigned D>
  friend constexpr auto operator*(dpp<A, B>, dpp<C, D>) noexcept;

  template <unsigned A, unsigned B, unsigned C, unsigned D>
  friend constexpr auto operator/(dpp<A, B>, dpp<C, D>) noexcept;
};

//////////////////////////////////////////////////////////////////////////////
template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator==(dpp<A, B> const a, dpp<C, D> const b) noexcept
{
  return isnan(a) || isnan(b) ? false :
    (a.exponent() == b.exponent()) && (a.mantissa() == b.mantissa());
}

template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator!=(dpp<A, B> const a, dpp<C, D> const b) noexcept
{
  return !operator==(a, b);
}

//////////////////////////////////////////////////////////////////////////////
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
    typename return_t::value_type m1(a.mantissa()), m2(b.mantissa());
    int e1(a.exponent()), e2(b.exponent());

    if (((e1 > e2) && equalize<return_t::exponent_bits>(m1, e1, m2, e2)) ||
      ((e2 > e1) && equalize<return_t::exponent_bits>(m2, e2, m1, e1)))
    {
      return false;
    }

    return m1 < m2;
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
  return !(a > b);
}

template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator>=(dpp<A, B> const a, dpp<C, D> const b) noexcept
{
  return !(a < b);
}

//////////////////////////////////////////////////////////////////////////////
template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator+(dpp<A, B> const a, dpp<C, D> const b) noexcept
{
  using return_t = dpp<(A > C ? A : C), (A > C ? B : D)>;

  if (isnan(a) || isnan(b))
  {
    return return_t{typename return_t::nan{}};
  }
  else
  {
    typename return_t::value_type m1(a.v_.m), m2(b.v_.m);
    int e1(a.v_.e), e2(b.v_.e);

    if (((e1 > e2) && equalize<return_t::exponent_bits>(m1, e1, m2, e2)) ||
      ((e2 > e1) && equalize<return_t::exponent_bits>(m2, e2, m1, e1)))
    {
      return return_t{typename return_t::nan{}};
    }

    // there can be no overflow
    return return_t(m1 + m2, e1);
  }
}

template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator-(dpp<A, B> const a, dpp<C, D> const b) noexcept
{
  using return_t = dpp<(A > C ? A : C), (A > C ? B : D)>;

  if (isnan(a) || isnan(b))
  {
    return return_t{typename return_t::nan{}};
  }
  else
  {
    typename return_t::value_type m1(a.v_.m), m2(b.v_.m);
    int e1(a.v_.e), e2(b.v_.e);

    if (((e1 > e2) && equalize<return_t::exponent_bits>(m1, e1, m2, e2)) ||
      ((e2 > e1) && equalize<return_t::exponent_bits>(m2, e2, m1, e1)))
    {
      return return_t{typename return_t::nan{}};
    }

    // there can be no overflow
    return return_t(m1 - m2, e1);
  }
}

template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator*(dpp<A, B> const a, dpp<C, D> const b) noexcept
{
  using return_t = dpp<(A > C ? A : C), (A > C ? B : D)>;

  return isnan(a) || isnan(b) ?
    return_t{typename return_t::nan{}} :
    return_t(typename return_t::doubled_t(a.v_.m) * b.v_.m, a.v_.e + b.v_.e);
}

template <unsigned A, unsigned B, unsigned C, unsigned D>
constexpr auto operator/(dpp<A, B> const a, dpp<C, D> const b) noexcept
{
  using return_t = dpp<(A > C ? A : C), (A > C ? B : D)>;

  if (isnan(a) || isnan(b) || !b.v_.m)
  {
    return return_t{typename return_t::nan{}};
  }
  else if (auto am(a.v_.m); am)
  {
    constexpr auto rmin(typename return_t::doubled_t(1) <<
      (bit_size<typename return_t::doubled_t>() - 1));
    constexpr auto rmax(-(rmin + 1));

    int e(a.v_.e - decimal_places<typename return_t::doubled_t>{} - b.v_.e);

    auto r(pow<10, typename return_t::doubled_t>(
      decimal_places<typename return_t::doubled_t>{}) / b.v_.m);

    if (am < 0)
    {
      am = -am;
      r = -r;
    }

    // fit r * am into doubled_t, avoid one divide, don't allow sign changes
    if (r > 0)
    {
      while (r > rmax / am)
      {
        r /= 10;
        ++e;
      }
    }
    else
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

//////////////////////////////////////////////////////////////////////////////
template <unsigned M, unsigned E>
constexpr auto abs(dpp<M, E> const p) noexcept
{
  return p.mantissa() < 0 ? -p : p;
}

//////////////////////////////////////////////////////////////////////////////
template <unsigned M, unsigned E>
constexpr bool isnan(dpp<M, E> const o) noexcept
{
  return -pow<2, int>(E - 1) == o.exponent();
}

//////////////////////////////////////////////////////////////////////////////
template <unsigned M, unsigned E>
constexpr auto sign(dpp<M, E> const o) noexcept
{
  auto const m(o.mantissa());

  return (m > 0) - (m < 0);
}

template <unsigned M, unsigned E>
constexpr auto ceil(dpp<M, E> const o) noexcept
{
  auto const t(trunc(o));

  return t + dpp<M, E>(t < o);
}

template <unsigned M, unsigned E>
constexpr auto floor(dpp<M, E> const o) noexcept
{
  auto const t(trunc(o));

  return t - dpp<M, E>(t > o);
}

template <unsigned M, unsigned E>
constexpr auto round(dpp<M, E> const o) noexcept
{
  if (!isnan(o) && (o.exponent() < 0))
  {
    constexpr dpp<M, E> c(5, -1);

    return trunc(o.mantissa() > 0 ? o + c : o - c);
  }
  else
  {
    return o;
  }
}

template <unsigned M, unsigned E>
constexpr auto frac(dpp<M, E> const o) noexcept
{
  return o - trunc(o);
}

template <unsigned M, unsigned E>
constexpr auto trunc(dpp<M, E> const o) noexcept
{
  if (auto e(o.exponent()); !isnan(o) && (e < 0))
  {
    auto m(o.mantissa());

    for (; m && e++; m /= 10);

    return dpp<M, E>(m, 0, typename dpp<M, E>::direct{});
  }
  else
  {
    return o;
  }
}

//////////////////////////////////////////////////////////////////////////////
using d64 = dpp<56, 8>;
using d32 = dpp<26, 6>;

//////////////////////////////////////////////////////////////////////////////
template <unsigned M, unsigned E>
constexpr std::optional<std::intmax_t> to_integral(dpp<M, E> const p) noexcept
{
  if (!isnan(p))
  {
    auto const r(p.mantissa());

    if (auto const e(p.exponent()); e < 0)
    {
      return r / pow<10, std::intmax_t>(-e);
    }
    else
    {
      if (auto const c(pow<10, std::intmax_t>(e)); c)
      {
        if (r > 0)
        {
          if (r <= std::numeric_limits<std::intmax_t>::max() / c)
          {
            return r * c;
          }
        }
        else
        {
          if (r >= std::numeric_limits<std::intmax_t>::min() / c)
          {
            return r * c;
          }
        }
      }
    }
  }

  return {};
}

template <typename T, typename It>
constexpr T to_decimal(It i, It const end) noexcept
{
  if (i == end)
  {
    return {typename T::nan{}};
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
        return {typename T::nan{}};
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
          return {typename T::nan{}};
      }

      break;
    }

    int e{};

    constexpr auto emin(-pow<2, int>(T::exponent_bits - 1));

    for (; i != end; i = std::next(i))
    {
      switch (*i)
      {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          if (positive)
          {
            if ((e > emin + 1) && (r <= rmax / 10))
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
            if ((e > emin + 1) && (r >= rmin / 10))
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
          return {typename T::nan{}};
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
constexpr T to_float(dpp<M, E> const p) noexcept
{
  return isnan(p) ? NAN : p.mantissa() * std::pow(T(10), p.exponent());
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
inline auto& operator<<(std::ostream& os, dpp<M, E> const& p)
{
  return os << to_string(p);
}

//////////////////////////////////////////////////////////////////////////////
namespace literals
{

constexpr auto operator "" _d32(char const* const s,
  std::size_t const N) noexcept
{
  return to_decimal<d32>(std::string_view(s, N));
}

constexpr auto operator "" _d64(char const* const s,
  std::size_t const N) noexcept
{
  return to_decimal<d64>(std::string_view(s, N));
}

}

}

#endif // DPP_HPP
