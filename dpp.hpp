#ifndef DPP_HPP
# define DPP_HPP
# pragma once

#include <cassert>

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

template <unsigned M, unsigned E>
class dpp;

template <unsigned M, unsigned E>
constexpr bool isnan(dpp<M, E> const&) noexcept;

template <unsigned M, unsigned E>
constexpr auto trunc(dpp<M, E> const&) noexcept;

template <typename T, typename S>
constexpr auto to_decimal(S const& s) noexcept ->
  decltype(std::cbegin(s), std::cend(s), T());

template <typename T, unsigned M, unsigned E>
constexpr T to_float(dpp<M, E> const&) noexcept;

template <unsigned M, unsigned E>
constexpr std::optional<std::intmax_t> to_integral(dpp<M, E> const&) noexcept;

template <unsigned M, unsigned E>
class dpp
{
public:
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
  constexpr static auto bit_size() noexcept
  {
    return 8 * sizeof(U);
  }

  struct
  {
    value_type e:E;
    value_type m:M;
  } v_{};

  template <int B, typename T = value_type>
  static constexpr T pow(unsigned e) noexcept
  {
    if (e)
    {
      T x(B);
      T y(1);

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

  template <unsigned B, typename T = value_type>
  static constexpr T log(T const n, unsigned const e = 0) noexcept
  {
    return pow<B>(e) < n ? log<B>(n, e + 1) : e;
  }

  static constexpr bool equalize(value_type& am, value_type& ae,
    value_type& bm, value_type& be) noexcept
  {
    constexpr auto emin(-pow<2>(E - 1));
    constexpr auto emax(-(emin + 1));

    // reserve one bit in case of overflow
    constexpr auto rmin(-pow<2>(bit_size<value_type>() - 2));
    constexpr auto rmax(-(rmin + 1));

    if (am > 0)
    {
      while ((ae != be) && (am <= rmax / 10))
      {
        // watch the nan
        if (ae > emin + 1)
        {
          --ae;
          am *= 10;
        }
        else
        {
          return true;
        }
      }
    }
    else if (am < 0)
    {
      while ((ae != be) && (am >= rmin / 10))
      {
        // watch the nan
        if (ae > emin + 1)
        {
          --ae;
          am *= 10;
        }
        else
        {
          return true;
        }
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

            if (bm <= rmax - 4)
            {
              bm += 4;
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

            if (bm >= rmin + 4)
            {
              bm -= 4;
            }

            bm /= 10;
          }
          else
          {
            return true;
          }
        }
      }
      else
      {
        be = ae;
      }
    }

    return false;
  }

  constexpr void normalize() noexcept
  {
    assert(!isnan(*this));

    constexpr auto emax(pow<2>(E - 1) - 1);

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
  }

public:
  constexpr dpp() noexcept = default;

  constexpr dpp(dpp const&) = default;
  constexpr dpp(dpp&&) = default;

  template <typename U,
    std::enable_if_t<std::is_integral_v<std::decay_t<U>>, int> = 0
  >
  constexpr dpp(U const m) noexcept :
    dpp(m, 0)
  {
  }

  template <typename U,
    std::enable_if_t<
      std::is_integral_v<U> ||
      std::is_same_v<U, __int128>,
      int
    > = 0
  >
  constexpr dpp(U m, value_type const e) noexcept
  {
    constexpr auto emin(-pow<2>(E - 1));
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

    constexpr auto mmin(-pow<2>(M - 1));
    constexpr auto mmax(-(mmin + 1));

    constexpr auto umin(U(1) << (bit_size<U>() - 1));
    constexpr auto umax(-(umin + 1));

    while (m < mmin)
    {
      if (v_.e <= emax - 1)
      {
        ++v_.e;

        if (m >= umin + 4)
        {
          m -= 4;
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

        if (m <= umax - 4)
        {
          m += 4;
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
  constexpr dpp(U f) noexcept
  {
    std::intmax_t r(f);
    f -= r;

    value_type e{};

    constexpr auto emin(std::numeric_limits<value_type>::min());

    constexpr auto rmin(std::numeric_limits<std::intmax_t>::min());
    constexpr auto rmax(std::numeric_limits<std::intmax_t>::max());

    if (r >= 0)
    {
      while (f)
      {
        if (int const d(f *= 10); (e > emin + 1) && (r <= rmax / 10))
        {
          if (r *= 10; r <= rmax - d)
          {
            r += d;
            f -= d;

            --e;

            continue;
          }
        }

        break;
      }
    }
    else if (r < 0)
    {
      while (f)
      {
        if (int const d(f *= 10); (e > emin + 1) && (r >= rmin / 10))
        {
          if (r *= 10; r >= rmin - d)
          {
            r += d;
            f -= d;

            --e;

            continue;
          }
        }

        break;
      }
    }

    *this = dpp(r, e);
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

  constexpr dpp(value_type const m, value_type const e, direct&&) noexcept :
    v_{.e = e, .m = m}
  {
  }

  struct nan{};

  constexpr dpp(nan&&) noexcept :
    v_{.e = -pow<2>(E - 1), .m = {}}
  {
  }

  struct unpack{};

  constexpr dpp(value_type const v, unpack&&) noexcept :
    v_{.e = v >> M, .m = v & (pow<2>(M) - 1)}
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
    assert(!isnan(*this));
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
  template <unsigned N, unsigned F>
  constexpr auto operator==(dpp<N, F> const& o) const noexcept
  {
    return isnan(*this) || isnan(o) ? false :
      (v_.e == o.v_.e) && (v_.m == o.v_.m);
  }

  template <unsigned N, unsigned F>
  constexpr auto operator!=(dpp<N, F> const& o) const noexcept
  {
    return !operator==<N, F>(o);
  }

  //
  constexpr auto operator<(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o))
    {
      return false;
    }
    else
    {
      auto const tmp(*this - o);

      return isnan(tmp) ? false : tmp.v_.m < 0;
    }
  }

  constexpr auto operator<=(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o))
    {
      return false;
    }
    else
    {
      auto const tmp(*this - o);

      return isnan(tmp) ? false : tmp.v_.m <= 0;
    }
  }

  constexpr auto operator>(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o))
    {
      return false;
    }
    else
    {
      auto const tmp(*this - o);

      return isnan(tmp) ? false : tmp.v_.m > 0;
    }
  }

  constexpr auto operator>=(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o))
    {
      return false;
    }
    else
    {
      auto const tmp(*this - o);

      return isnan(tmp) ? false : tmp.v_.m >= 0;
    }
  }

  //
  template <unsigned N, unsigned F>
  constexpr auto operator<(dpp<N, F> const& o) const noexcept
  {
    using result_t = dpp<(M > N ? M : N), (M > N ? E : F)>;

    if constexpr (std::is_same_v<dpp, result_t>)
    {
      return *this < dpp(o);
    }
    else
    {
      return result_t(*this) < o;
    }
  }

  template <unsigned N, unsigned F>
  constexpr auto operator<=(dpp<N, F> const& o) const noexcept
  {
    using result_t = dpp<(M > N ? M : N), (M > N ? E : F)>;

    if constexpr (std::is_same_v<dpp, result_t>)
    {
      return *this <= dpp(o);
    }
    else
    {
      return result_t(*this) <= o;
    }
  }

  template <unsigned N, unsigned F>
  constexpr auto operator>(dpp<N, F> const& o) const noexcept
  {
    using result_t = dpp<(M > N ? M : N), (M > N ? E : F)>;

    if constexpr (std::is_same_v<dpp, result_t>)
    {
      return *this > dpp(o);
    }
    else
    {
      return result_t(*this) > o;
    }
  }

  template <unsigned N, unsigned F>
  constexpr auto operator>=(dpp<N, F> const& o) const noexcept
  {
    using result_t = dpp<(M > N ? M : N), (M > N ? E : F)>;

    if constexpr (std::is_same_v<dpp, result_t>)
    {
      return *this >= dpp(o);
    }
    else
    {
      return result_t(*this) >= o;
    }
  }

  //
  constexpr auto operator+() const noexcept
  {
    return *this;
  }

  constexpr auto operator-() const noexcept
  {
    // we need to do it like this as negating the mantissa can overflow 
    return dpp(-mantissa(), exponent());
  }

  //
  constexpr auto operator+(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o))
    {
      return dpp{nan{}};
    }
    else
    {
      value_type m1(v_.m), m2(o.v_.m), e1(v_.e), e2(o.v_.e);

      if (((e1 > e2) && equalize(m1, e1, m2, e2)) || 
        ((e2 > e1) && equalize(m2, e2, m1, e1)))
      {
        return dpp{nan{}};
      }

      // there can be no overflow
      return dpp(m1 + m2, e1);
    }
  }

  constexpr auto operator-(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o))
    {
      return dpp{nan{}};
    }
    else
    {
      value_type m1(v_.m), m2(o.v_.m), e1(v_.e), e2(o.v_.e);

      if (((e1 > e2) && equalize(m1, e1, m2, e2)) || 
        ((e2 > e1) && equalize(m2, e2, m1, e1)))
      {
        return dpp{nan{}};
      }

      // there can be no overflow
      return dpp(m1 - m2, e1);
    }
  }

  //
  constexpr auto operator*(dpp const& o) const noexcept
  {
    return isnan(*this) || isnan(o) ?  dpp{nan{}} :
      dpp(doubled_t(v_.m) * o.v_.m, v_.e + o.v_.e);
  }

  constexpr auto operator/(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o) || !o.v_.m)
    {
      return dpp{nan{}};
    }
    else if (v_.m)
    {
      constexpr auto emin(-pow<2>(E - 1));
      constexpr auto emax(-(emin + 1));

      constexpr auto rmin(doubled_t(1) << (bit_size<doubled_t>() - 1));
      constexpr auto rmax(-(rmin + 1));

      value_type e(-dpp::dpp::decimal_places<doubled_t>{});

      auto r(pow<10, doubled_t>(-e) / o.v_.m);

      // fit r * v_.m into doubled_t, avoid one divide
      if (r > 0)
      {
        while (r > rmax / v_.m)
        {
          if (e <= emax - 1)
          {
            ++e;
/*
            if (r <= rmax - 5)
            {
              r += 5;
            }
*/
            r /= 10;
          }
          else
          {
            return dpp{nan{}};
          }
        }
      }
      else if (r < 0)
      {
        while (r < rmin / v_.m)
        {
          if (e <= emax - 1)
          {
            ++e;
/*
            if (r >= rmin + 5)
            {
              r -= 5;
            }
*/
            r /= 10;
          }
          else
          {
            return dpp{nan{}};
          }
        }
      }

      return dpp(v_.m * r, exponent() + e - o.exponent());
    }
    else
    {
      return dpp{};
    }
  }

  //
  template <unsigned N, unsigned F>
  constexpr auto operator+(dpp<N, F> const& o) const noexcept
  {
    using result_t = dpp<(M > N ? M : N), (M > N ? E : F)>;

    if constexpr (std::is_same_v<dpp, result_t>)
    {
      return *this + dpp(o);
    }
    else
    {
      return result_t(*this) + o;
    }
  }

  template <unsigned N, unsigned F>
  constexpr auto operator-(dpp<N, F> const& o) const noexcept
  {
    using result_t = dpp<(M > N ? M : N), (M > N ? E : F)>;

    if constexpr (std::is_same_v<dpp, result_t>)
    {
      return *this - dpp(o);
    }
    else
    {
      return result_t(*this) - o;
    }
  }

  template <unsigned N, unsigned F>
  constexpr auto operator*(dpp<N, F> const& o) const noexcept
  {
    using result_t = dpp<(M > N ? M : N), (M > N ? E : F)>;

    if constexpr (std::is_same_v<dpp, result_t>)
    {
      return *this * dpp(o);
    }
    else
    {
      return result_t(*this) * o;
    }
  }

  template <unsigned N, unsigned F>
  constexpr auto operator/(dpp<N, F> const& o) const noexcept
  {
    using result_t = dpp<(M > N ? M : N), (M > N ? E : F)>;

    if constexpr (std::is_same_v<dpp, result_t>)
    {
      return *this / dpp(o);
    }
    else
    {
      return result_t(*this) / o;
    }
  }

  //
  constexpr auto& operator+=(dpp const& o) noexcept
  {
    return *this = *this + o;
  }

  constexpr auto& operator-=(dpp const& o) noexcept
  {
    return *this = *this - o;
  }

  constexpr auto& operator*=(dpp const& o) noexcept
  {
    return *this = *this * o;
  }

  constexpr auto& operator/=(dpp const& o) noexcept
  {
    return *this = *this / o;
  }

  //
  template <unsigned N, unsigned F>
  constexpr auto& operator+=(dpp<N, F> const& o) noexcept
  {
    using result_t = dpp<(M > N ? M : N), (M > N ? E : F)>;

    if constexpr (std::is_same_v<dpp, result_t>)
    {
      return *this = *this + dpp(o);
    }
    else
    {
      return *this = result_t(*this) + o;
    }
  }

  template <unsigned N, unsigned F>
  constexpr auto& operator-=(dpp<N, F> const& o) noexcept
  {
    using result_t = dpp<(M > N ? M : N), (M > N ? E : F)>;

    if constexpr (std::is_same_v<dpp, result_t>)
    {
      return *this = *this - dpp(o);
    }
    else
    {
      return *this = result_t(*this) - o;
    }
  }

  template <unsigned N, unsigned F>
  constexpr auto& operator*=(dpp<N, F> const& o) noexcept
  {
    using result_t = dpp<(M > N ? M : N), (M > N ? E : F)>;

    if constexpr (std::is_same_v<dpp, result_t>)
    {
      return *this = *this * dpp(o);
    }
    else
    {
      return *this = result_t(*this) * o;
    }
  }

  template <unsigned N, unsigned F>
  constexpr auto& operator/=(dpp<N, F> const& o) noexcept
  {
    using result_t = dpp<(M > N ? M : N), (M > N ? E : F)>;

    if constexpr (std::is_same_v<dpp, result_t>)
    {
      return *this = *this / dpp(o);
    }
    else
    {
      return *this = result_t(*this) / o;
    }
  }

  //
  constexpr auto exponent() const noexcept
  {
    return v_.e;
  }

  constexpr auto mantissa() const noexcept
  {
    return v_.m;
  }

  constexpr auto packed() const noexcept
  {
    return (v_.e << M) | v_.m;
  }

  friend constexpr bool isnan<M, E>(dpp<M, E> const&) noexcept;

  friend constexpr std::optional<std::intmax_t> to_integral<M, E>(
    dpp<M, E> const&) noexcept;

  friend constexpr auto trunc<M, E>(dpp<M, E> const& o) noexcept;
};

//////////////////////////////////////////////////////////////////////////////
template <unsigned M, unsigned E>
constexpr auto abs(dpp<M, E> const& p) noexcept
{
  return p.mantissa() < 0 ? -p : p;
}

//////////////////////////////////////////////////////////////////////////////
template <unsigned M, unsigned E>
constexpr bool isnan(dpp<M, E> const& o) noexcept
{
  return -dpp<M, E>::template pow<2>(E - 1) == o.v_.e;
}

//////////////////////////////////////////////////////////////////////////////
template <unsigned M, unsigned E>
constexpr auto sign(dpp<M, E> const& o) noexcept
{
  auto const m(o.mantissa());

  return (m > 0) - (m < 0);
}

template <unsigned M, unsigned E>
constexpr auto ceil(dpp<M, E> const& o) noexcept
{
  auto const t(trunc(o));

  return t + (t < o);
}

template <unsigned M, unsigned E>
constexpr auto floor(dpp<M, E> const& o) noexcept
{
  auto const t(trunc(o));

  return t - (t > o);
}

template <unsigned M, unsigned E>
constexpr auto round(dpp<M, E> const& o) noexcept
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
constexpr auto trunc(dpp<M, E> const& o) noexcept
{
  if (auto e(o.exponent()); !isnan(o) && (e < 0))
  {
    auto m(o.mantissa());

    for (; m && e++; m /= 10);

    return dpp<M, E>(m, 0);
  }
  else
  {
    return o;
  }
}

//////////////////////////////////////////////////////////////////////////////
using dec64 = dpp<56, 8>;
using dec32 = dpp<26, 6>;

//////////////////////////////////////////////////////////////////////////////
template <unsigned M, unsigned E>
constexpr std::optional<std::intmax_t> to_integral(
  dpp<M, E> const& p) noexcept
{
  if (!isnan(p))
  {
    auto const r(p.mantissa());

    if (auto const e(p.exponent()); e < 0)
    {
      return r / dpp<M, E>::template pow<10, std::intmax_t>(-e);
    }
    else
    {
      if (auto const c(dpp<M, E>::template pow<10, std::intmax_t>(e)); c)
      {
        if (r > 0)
        {
          if (r <= std::numeric_limits<std::intmax_t>::max() / c)
          {
            return r * c;
          }
        }
        else if (r < 0)
        {
          if (r >= std::numeric_limits<std::intmax_t>::min() / c)
          {
            return r * c;
          }
        }
        else
        {
          return 0;
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

    typename T::value_type e{};

    constexpr auto emin(std::numeric_limits<typename T::value_type>::min());

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
constexpr T to_float(dpp<M, E> const& p) noexcept
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

    if (p < 0)
    {
      p = -p;

      r.append(1, '-');
    }

    {
      auto const t(trunc(p));

      r.append(std::to_string(t.mantissa())).append(t.exponent(), '0');

      p -= t;
    }

    if (auto const m(p.mantissa()); m)
    {
      if (auto const tmp(std::to_string(m)); -p.exponent() >= tmp.size())
      {
        r.append(1, '.').append(-p.exponent() - tmp.size(), '0').append(tmp);
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
  return to_decimal<dec32>(std::string_view(s, N));
}

constexpr auto operator "" _d64(char const* const s,
  std::size_t const N) noexcept
{
  return to_decimal<dec64>(std::string_view(s, N));
}

}

}

#endif // DPP_HPP
