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
constexpr auto sign(dpp<M, E> const&) noexcept;

template <unsigned M, unsigned E>
constexpr dpp<M, E> trunc(dpp<M, E> const&) noexcept;

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

  static constexpr auto round_mantissa(dpp& tmp) noexcept
  {
    if ((tmp.v_.m > 0) && (tmp.v_.m <= pow<2>(M - 1) - 1 - 5))
    {
      tmp.v_.m += 5;
    }
    else if ((tmp.v_.m < 0) && (tmp.v_.m >= -pow<2>(M - 1) + 5))
    {
      tmp.v_.m -= 5;
    }
  }

  static constexpr void equalize(dpp& a, dpp& b) noexcept
  {
    //a.v_.m *= pow<10>(a.v_.e - b.v_.e);
    //a.v_.e = b.v_.e;

    switch ((a.v_.m > 0) - (a.v_.m < 0))
    {
      case -1:
        while ((a.v_.m >= -pow<2>(M - 1) / 10) &&
          (a.v_.e != b.v_.e))
        {
          if (a.decrease_exponent())
          {
            break;
          }
          else
          {
            a.v_.m *= 10;
          }
        }

        break;

      case 0:
        a.v_.e = b.v_.e;

        break;

      case 1:
        while ((a.v_.m <= (pow<2>(M - 1) - 1) / 10) &&
          (a.v_.e != b.v_.e))
        {
          if (a.decrease_exponent())
          {
            break;
          }
          else
          {
            a.v_.m *= 10;
          }
        }

        break;
    }

    if (!isnan(a))
    {
      while (a.v_.e != b.v_.e)
      {
        round_mantissa(b);

        if (b.increase_exponent())
        {
          break;
        }
        else
        {
          b.v_.m /= 10;
        }
      }
    }

/*
    if (!isnan(a) && (a.v_.e != b.v_.e))
    {
      round_mantissa(b);

      auto const d(a.v_.e - b.v_.e);

      b.v_.m /= pow<10>(d);
      b.increase_exponent(d);
    }
*/
  }

  static constexpr bool fix_floats(dpp& a, dpp& b) noexcept
  {
    // both floats need to be fixed to preserve the identical exponents
    round_mantissa(a);
    a.v_.m /= 10;

    round_mantissa(b);
    b.v_.m /= 10;

    return a.increase_exponent() || b.increase_exponent();
  }

  static constexpr auto add_prep(dpp tmp1, dpp tmp2) noexcept
  {
    if (tmp1.v_.e > tmp2.v_.e)
    {
      equalize(tmp1, tmp2);
    }
    else if (tmp2.v_.e > tmp1.v_.e)
    {
      equalize(tmp2, tmp1);
    }

    if (auto const s(sign(tmp1)); s == sign(tmp2))
    {
      switch (s)
      {
        case -1:
        {
          constexpr auto min(-pow<2>(M - 1));

          while (tmp1.v_.m < min - tmp2.v_.m)
          {
            if (fix_floats(tmp1, tmp2))
            {
              break;
            }
          }

          break;
        }

        case 1:
        {
          constexpr auto max(pow<2>(M - 1) - 1);

          while (tmp1.v_.m > max - tmp2.v_.m)
          {
            if (fix_floats(tmp1, tmp2))
            {
              break;
            }
          }

          break;
        }

        default:
          break;
      }
    }

    return std::pair(tmp1, tmp2);
  }

  static constexpr auto sub_prep(dpp tmp1, dpp tmp2) noexcept
  {
    if (tmp1.v_.e > tmp2.v_.e)
    {
      equalize(tmp1, tmp2);
    }
    else if (tmp2.v_.e > tmp1.v_.e)
    {
      equalize(tmp2, tmp1);
    }

    if (auto const s(sign(tmp1)); s != sign(tmp2))
    {
      switch (s)
      {
        case -1:
        {
          constexpr auto min(-pow<2>(M - 1));

          while (tmp1.v_.m < min + tmp2.v_.m)
          {
            if (fix_floats(tmp1, tmp2))
            {
              break;
            }
          }

          break;
        }

        case 1:
        {
          constexpr auto max(pow<2>(M - 1) - 1);

          while (tmp1.v_.m > max + tmp2.v_.m)
          {
            if (fix_floats(tmp1, tmp2))
            {
              break;
            }
          }

          break;
        }

        default:
          break;
      }
    }

    return std::pair(tmp1, tmp2);
  }

  constexpr bool decrease_exponent(int const e = 1) noexcept
  {
    if ((v_.e > -pow<2>(E - 1) + e) && (v_.e <= pow<2>(E - 1) - 1 + e))
    {
      v_.e -= e;

      return false;
    }
    else
    {
      *this = dpp{nan{}};

      return true;
    }
  }

  constexpr bool increase_exponent(int const e = 1) noexcept
  {
    if ((v_.e > -pow<2>(E - 1) - e) && (v_.e <= pow<2>(E - 1) - 1 - e))
    {
      v_.e += e;

      return false;
    }
    else
    {
      *this = dpp{nan{}};

      return true;
    }
  }

  constexpr void normalize() noexcept
  {
    assert(!isnan(*this));

    if (v_.m)
    {
      for (; !((v_.m % 10) || increase_exponent()); v_.m /= 10);
    }
  }

public:
  constexpr dpp() noexcept = default;

  constexpr dpp(dpp const&) = default;
  constexpr dpp(dpp&&) = default;

  template <typename U,
    std::enable_if_t<std::is_integral_v<std::decay_t<U>>, int> = 0
  >
  constexpr dpp(U m) noexcept
  {
    while (m > pow<2>(M - 1) - 1)
    {
      if (m <= std::numeric_limits<value_type>::max() - 5)
      {
        m += 5;
      }

      m /= 10;

      if (increase_exponent())
      {
        return;
      }
    }

    while (m < -pow<2>(M - 1))
    {
      if (m >= std::numeric_limits<value_type>::min() + 5)
      {
        m -= 5;
      }

      m /= 10;

      if (increase_exponent())
      {
        return;
      }
    }

    v_.m = m;

    normalize();
  }

  constexpr dpp(std::intmax_t m, value_type const e) noexcept
  {
    if ((e <= pow<2>(E - 1) - 1) && (e >= -pow<2>(E - 1)))
    {
      v_.e = e;
    }
    else
    {
      *this = dpp{nan{}};

      return;
    }

    if (m > 0)
    {
      while (m > pow<2>(M - 1) - 1)
      {
        if (m <= std::numeric_limits<value_type>::max() - 5)
        {
          m += 5;
        }

        m /= 10;

        if (increase_exponent())
        {
          return;
        }
      }
    }
    else if (m < 0)
    {
      while (m < -pow<2>(M - 1))
      {
        if (m >= std::numeric_limits<value_type>::min() + 5)
        {
          m -= 5;
        }

        m /= 10;

        if (increase_exponent())
        {
          return;
        }
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

    constexpr auto emin(std::numeric_limits<value_type>::min());

    value_type e{};

    if (r >= 0)
    {
      constexpr auto rmax(std::numeric_limits<std::intmax_t>::max());

      while (f)
      {
        if (int const d(f *= 10); (e >= emin + 1) && (r <= rmax / 10))
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
      constexpr auto rmin(std::numeric_limits<std::intmax_t>::min());

      while (f)
      {
        if (int const d(f *= 10); (e >= emin + 1) && (r >= rmin / 10))
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
  constexpr auto operator==(dpp const& o) const noexcept
  {
    return isnan(*this) || isnan(o) ? false :
      (v_.e == o.v_.e) && (v_.m == o.v_.m);
  }

  constexpr auto operator!=(dpp const& o) const noexcept
  {
    return !operator==(o);
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
    auto tmp(*this);

    if (tmp.v_.m == -pow<2>(M - 1))
    {
      round_mantissa(tmp);

      tmp.v_.m /= 10;

      tmp.increase_exponent();
    }

    tmp.v_.m = -tmp.v_.m;

    return tmp;
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
      auto [tmp1, tmp2](add_prep(*this, o));

      if (!isnan(tmp1) && !isnan(tmp2))
      {
        tmp1.v_.m += tmp2.v_.m;

        tmp1.normalize();
      }

      return tmp1;
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
      auto [tmp1, tmp2](sub_prep(*this, o));

      if (!isnan(tmp1) && !isnan(tmp2))
      {
        tmp1.v_.m -= tmp2.v_.m;

        tmp1.normalize();
      }

      return tmp1;
    }
  }

  //
  constexpr auto operator*(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o))
    {
      return dpp{nan{}};
    }
    else
    {
      auto tmp(*this);

      if (tmp.increase_exponent(o.v_.e))
      {
        return dpp{nan{}};
      }
      else
      {
        auto r(doubled_t(tmp.v_.m) * o.v_.m);

        // fit into target mantissa
        while (r < -pow<2>(M - 1))
        {
          if (r >= -pow<2, doubled_t>(bit_size<doubled_t>() - 1) + 5)
          {
            r -= 5;
          }

          r /= 10;

          if (tmp.increase_exponent())
          {
            return dpp{nan{}};
          }
        }

        while (r > pow<2>(M - 1) - 1)
        {
          if (r <= pow<2, doubled_t>(bit_size<doubled_t>() - 1) - 1 - 5)
          {
            r += 5;
          }

          r /= 10;

          if (tmp.increase_exponent())
          {
            return dpp{nan{}};
          }
        }

        tmp.v_.m = r;
      }

      tmp.normalize();

      return tmp;
    }
  }

  constexpr auto operator/(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o) || !o.v_.m)
    {
      return dpp{nan{}};
    }
    else
    {
      auto tmp(*this);

      if (constexpr auto e(dpp::dpp::decimal_places<doubled_t>{});
        tmp.decrease_exponent(e) || tmp.decrease_exponent(o.v_.e))
      {
        return dpp{nan{}};
      }
      else
      {
        auto r(pow<10, doubled_t>(e) / o.v_.m);

        constexpr auto rmin(pow<-2, doubled_t>(bit_size<doubled_t>() - 1));
        constexpr auto rmax(-(rmin + 1));

        if (r > 0)
        {
          while (r > rmax / tmp.v_.m)
          {
            if (r <= rmax - 5)
            {
              r += 5;
            }

            r /= 10;

            if (tmp.increase_exponent())
            {
              return dpp{nan{}};
            }
          }
        }
        else if (r < 0)
        {
          while (r < rmin / tmp.v_.m)
          {
            if (r >= rmin + 5)
            {
              r -= 5;
            }

            r /= 10;

            if (tmp.increase_exponent())
            {
              return dpp{nan{}};
            }
          }
        }

        r *= tmp.v_.m;

        while (r > pow<2>(M - 1) - 1)
        {
          if (r <= rmax - 5)
          {
            r += 5;
          }

          r /= 10;

          if (tmp.increase_exponent())
          {
            return dpp{nan{}};
          }
        }

        while (r < -pow<2>(M - 1))
        {
          if (r >= rmin + 5)
          {
            r -= 5;
          }

          r /= 10;

          if (tmp.increase_exponent())
          {
            return dpp{nan{}};
          }
        }

        tmp.v_.m = r;
      }

      tmp.normalize();

      return tmp;
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
    return (v_.m << E) | v_.e;
  }

  //
  friend constexpr bool isnan<M, E>(dpp<M, E> const&) noexcept;

  friend constexpr auto sign<M, E>(dpp<M, E> const&) noexcept;

  friend constexpr std::optional<std::intmax_t> to_integral<M, E>(
    dpp<M, E> const&) noexcept;

  friend constexpr dpp<M, E> trunc<M, E>(dpp<M, E> const& o) noexcept;
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
  return (o.v_.m > 0) - (o.v_.m < 0);
}

template <unsigned M, unsigned E>
constexpr auto ceil(dpp<M, E> const& x) noexcept
{
  auto const t(trunc(x));

  return t + (t < x);
}

template <unsigned M, unsigned E>
constexpr auto floor(dpp<M, E> const& x) noexcept
{
  auto const t(trunc(x));

  return t - (t > x);
}

template <unsigned M, unsigned E>
constexpr auto round(dpp<M, E> const& x) noexcept
{
  if (x.exponent() < 0)
  {
    constexpr dpp<M, E> c(5, -1);

    return trunc(x.mantissa() > 0 ? x + c : x - c);
  }
  else
  {
    return x;
  }
}

template <unsigned M, unsigned E>
constexpr dpp<M, E> trunc(dpp<M, E> const& o) noexcept
{
  if (auto const e(o.exponent()); !isnan(o) && (e < 0))
  {
    if (auto tmp(o); tmp.increase_exponent(-e))
    {
      return dpp<M, E>{typename dpp<M, E>::nan{}};
    }
    else
    {
      tmp.v_.m /= dpp<M, E>::template pow<10>(-e);

      tmp.normalize();

      return tmp;
    }
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
        switch ((r > 0) - (r < 0))
        {
          case -1:
            if (r >= std::numeric_limits<std::intmax_t>::min() / c)
            {
              return r * c;
            }

            break;

          case 0:
            return 0;

          case 1:
            if (r <= std::numeric_limits<std::intmax_t>::max() / c)
            {
              return r * c;
            }

            break;
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

    constexpr auto max(std::numeric_limits<std::intmax_t>::max());

    for (; i != end; i = std::next(i))
    {
      switch (*i)
      {
        case '.':
          i = std::next(i);
          break;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          if (r <= max / 10)
          {
            r *= 10;

            if (auto const d(*i - '0'); r <= max - d)
            {
              r += d;

              continue;
            }
          }

          break;

        case '\0':
        {
          auto const tmp(T(r, 0));
          return positive ? tmp : -tmp;
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
          if ((e >= emin + 1) && (r <= max / 10))
          {
            r *= 10;

            if (auto const d(*i - '0'); r <= max - d)
            {
              r += d;
              --e;

              continue;
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

    auto const tmp(T(r, e));
    return positive ? tmp : -tmp;
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

  std::string r;

  if (p < 0)
  {
    p = -p;

    r.append(1, '-');
  }

  if (auto const o(to_integral(p)); o.has_value())
  {
    auto const v(o.value());

    r.append(std::to_string(v));

    p -= v;
  }
  else
  {
    return {"nan", 3};
  }

  if (p.mantissa())
  {
    auto const tmp(std::to_string(p.mantissa()));

    r.append(1, '.').append(-p.exponent() - tmp.size(), '0').append(tmp);
  }

  return r;
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
