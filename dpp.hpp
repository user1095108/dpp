#ifndef DPP_HPP
# define DPP_HPP
# pragma once

#include <cstdint>

#include <ostream>

#include <type_traits>

namespace dpp
{

template <unsigned M, unsigned E>
class dpp
{
public:
  using value_type = std::conditional_t<M + E <= 16, std::int16_t,
    std::conditional_t<M + E <= 32, std::int32_t,
      std::conditional_t<M + E <= 64, std::int64_t, void>
    >
  >;

private:
  struct
  {
    value_type m:M;
    value_type e:E;
  } v_;

  constexpr void normalize() noexcept
  {
    if (v_.m)
    {
      for (; !(v_.m % 10); v_.m /= 10)
      {
        ++v_.e;
      }
    }
  }

  template <value_type B, typename T = value_type>
  static constexpr auto pow(value_type e) noexcept
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

  template <unsigned B>
  static constexpr value_type log(value_type const n,
    value_type const e = 0) noexcept
  {
    return pow<B>(e) < n ? log<B>(n, e + 1) : e;
  }

  static constexpr auto equalize(dpp a, dpp& b) noexcept
  {
    //a.v_.m *= pow<10>(a.v_.e - b.v_.e);
    //a.v_.e = b.v_.e;

    while ((std::abs(a.v_.m) < (pow<2>(M - 1) - 1) / 10) &&
      (a.v_.e != b.v_.e))
    {
      a.v_.m *= 10;
      --a.v_.e;
    }

    if (a.v_.e != b.v_.e)
    {
      b.v_.m /= pow<10>(a.v_.e - b.v_.e);
      b.v_.e = a.v_.e;
    }

    return a;
  }

  static constexpr auto add_prep(dpp tmp1, dpp tmp2) noexcept
  {
    if (tmp1.v_.e > tmp2.v_.e)
    {
      tmp1 = equalize(tmp1, tmp2);
    }
    else if (tmp2.v_.e > tmp1.v_.e)
    {
      tmp2 = equalize(tmp2, tmp1);
    }

    if (tmp1.sign() == tmp2.sign())
    {
      if (1 == tmp1.sign())
      {
        constexpr auto max(pow<2>(M - 1) - 1);

        while ((tmp1.v_.m > max - tmp2.v_.m) ||
          (tmp2.v_.m > max - tmp1.v_.m))
        {
          tmp1.v_.m /= 10;
          tmp2.v_.m /= 10;

          ++tmp1.v_.e;
          ++tmp2.v_.e;
        }
      }
      else if (-1 == tmp1.sign())
      {
        constexpr auto min(-pow<2>(M - 1));

        while ((tmp1.v_.m < min - tmp2.v_.m) ||
          (tmp2.v_.m < min - tmp1.v_.m))
        {
          tmp1.v_.m /= 10;
          tmp2.v_.m /= 10;

          ++tmp1.v_.e;
          ++tmp2.v_.e;
        }
      }
    }

    return std::pair(tmp1, tmp2);
  }

  static constexpr auto sub_prep(dpp tmp1, dpp tmp2) noexcept
  {
    if (tmp1.v_.e > tmp2.v_.e)
    {
      tmp1 = equalize(tmp1, tmp2);
    }
    else if (tmp2.v_.e > tmp1.v_.e)
    {
      tmp2 = equalize(tmp2, tmp1);
    }

    if (tmp1.sign() != tmp2.sign())
    {
      if (1 == tmp1.sign())
      {
        constexpr auto max(pow<2>(M - 1) - 1);

        while ((tmp1.v_.m > max + tmp2.v_.m) ||
          (tmp2.v_.m > max + tmp1.v_.m))
        {
          tmp1.v_.m /= 10;
          tmp2.v_.m /= 10;

          ++tmp1.v_.e;
          ++tmp2.v_.e;
        }
      }
      else if (-1 == tmp1.sign())
      {
        constexpr auto min(-pow<2>(M - 1));

        while ((tmp1.v_.m < min + tmp2.v_.m) ||
          (tmp2.v_.m < min + tmp1.v_.m))
        {
          tmp1.v_.m /= 10;
          tmp2.v_.m /= 10;

          ++tmp1.v_.e;
          ++tmp2.v_.e;
        }
      }
    }

    return std::pair(tmp1, tmp2);
  }

public:
  constexpr dpp() noexcept :
    v_{}
  {
  }

  template <typename U,
    std::enable_if_t<std::is_integral_v<std::decay_t<U>>, int> = 0
  >
  constexpr dpp(U&& v) noexcept :
    v_{v, {}}
  {
    normalize();
  }

  constexpr dpp(value_type const m, value_type const e) noexcept :
    v_{m, e}
  {
    normalize();
  }

  struct nan{};

  constexpr dpp(nan&&) noexcept :
    v_{{}, -pow<2>(E - 1)}
  {
  }

  struct val{};

  constexpr dpp(value_type const v, val&&) noexcept :
    v_{v >> E, v & (pow<2>(E) - 1)}
  {
  }

  constexpr auto exponent() const noexcept
  {
    return v_.e;
  }

  constexpr auto mantissa() const noexcept
  {
    return v_.m;
  }

  constexpr auto value() const noexcept
  {
    return (v_.m << E) | v_.e;
  }

  constexpr auto sign() const noexcept
  {
    return (v_.m > 0) - (v_.m < 0);
  }

  constexpr bool is_nan() const noexcept
  {
    return v_.e == -pow<2>(E - 1);
  }

  //
  constexpr explicit operator bool() noexcept
  {
    return is_nan() || v_.m;
  }

  constexpr explicit operator value_type() noexcept
  {
    auto r(v_.m);

    if (v_.e < 0)
    {
      r /= pow<10>(-v_.e);
    }
    else
    {
      r *= pow<10>(v_.e);
    }

    return r;
  }

  //
  constexpr auto operator==(dpp const& o) noexcept
  {
    if (is_nan() || o.is_nan())
    {
      return false;
    }
    else
    {
      return (v_.m == o.v_.m) && (v_.e == o.v_.e);
    }
  }

  constexpr auto operator!=(dpp const& o) noexcept
  {
    return !operator==(o);
  }

  //
  constexpr auto operator<(dpp const& o) noexcept
  {
    if (is_nan() || o.is_nan())
    {
      return false;
    }
    else
    {
      dpp tmp1(*this);
      dpp tmp2(o);

      if (tmp1.v_.e > tmp2.v_.e)
      {
        tmp1 = equalize(tmp1, tmp2);

        return tmp1.v_.m < tmp2.v_.m;
      }
      else if (tmp2.v_.e > tmp1.v_.e)
      {
        tmp2 = equalize(tmp2, tmp1);

        return tmp1.v_.m < tmp2.v_.m;
      }
      else
      {
        return tmp1.v_.m < tmp2.v_.m;
      }
    }
  }

  constexpr auto operator<=(dpp const& o) noexcept
  {
    if (is_nan() || o.is_nan())
    {
      return false;
    }
    else
    {
      dpp tmp1(*this);
      dpp tmp2(o);

      if (tmp1.v_.e > tmp2.v_.e)
      {
        tmp1 = equalize(tmp1, tmp2);

        return tmp1.v_.m <= tmp2.v_.m;
      }
      else if (tmp2.v_.e > tmp1.v_.e)
      {
        tmp2 = equalize(tmp2, tmp1);

        return tmp1.v_.m <= tmp2.v_.m;
      }
      else
      {
        return tmp1.v_.m <= tmp2.v_.m;
      }
    }
  }

  constexpr auto operator>(dpp const& o) noexcept
  {
    if (is_nan() || o.is_nan())
    {
      return false;
    }
    else
    {
      dpp tmp1(*this);
      dpp tmp2(o);

      if (tmp1.v_.e > tmp2.v_.e)
      {
        tmp1 = equalize(tmp1, tmp2);

        return tmp1.v_.m > tmp2.v_.m;
      }
      else if (tmp2.v_.e > tmp1.v_.e)
      {
        tmp2 = equalize(tmp2, tmp1);

        return tmp1.v_.m > tmp2.v_.m;
      }
      else
      {
        return tmp1.v_.m > tmp2.v_.m;
      }
    }
  }

  constexpr auto operator>=(dpp const& o) noexcept
  {
    if (is_nan() || o.is_nan())
    {
      return false;
    }
    else
    {
      dpp tmp1(*this);
      dpp tmp2(o);

      if (tmp1.v_.e > tmp2.v_.e)
      {
        tmp1 = equalize(tmp1, tmp2);

        return tmp1.v_.m >= tmp2.v_.m;
      }
      else if (tmp2.v_.e > tmp1.v_.e)
      {
        tmp2 = equalize(tmp2, tmp1);

        return tmp1.v_.m <= tmp2.v_.m;
      }
      else
      {
        return tmp1.v_.m >= tmp2.v_.m;
      }
    }
  }

  //
  constexpr auto operator+() noexcept
  {
    return *this;
  }

  constexpr auto operator-() const noexcept
  {
    auto tmp(*this);

    tmp.v_.m = -tmp.v_.m;

    return tmp;
  }

  //
  constexpr auto operator+(dpp const& o) const noexcept
  {
    if (is_nan() || o.is_nan())
    {
      return dpp{nan{}};
    }
    else
    {
      auto [tmp1, tmp2](add_prep(*this, o));

      tmp1.v_.m += tmp2.v_.m;
      tmp1.normalize();

      return tmp1;
    }
  }

  constexpr auto operator-(dpp const& o) const noexcept
  {
    if (is_nan() || o.is_nan())
    {
      return dpp{nan{}};
    }
    else
    {
      auto [tmp1, tmp2](sub_prep(*this, o));

      tmp1.v_.m -= tmp2.v_.m;
      tmp1.normalize();

      return tmp1;
    }
  }

  //
  constexpr auto& operator+=(dpp const& o) noexcept
  {
     if (is_nan() || o.is_nan())
    {
      return *this = dpp{nan{}};
    }
    else
    {
      auto [tmp1, tmp2](add_prep(*this, o));

      tmp1.v_.m += tmp2.v_.m;
      tmp1.normalize();

      return *this = tmp1;
    }
  }

  constexpr auto& operator-=(dpp const& o) noexcept
  {
     if (is_nan() || o.is_nan())
    {
      return *this = dpp{nan{}};
    }
    else
    {
      auto [tmp1, tmp2](sub_prep(*this, o));

      tmp1.v_.m -= tmp2.v_.m;
      tmp1.normalize();

      return *this = tmp1;
    }
  }

  //
  constexpr auto operator*(dpp const& o) const noexcept
  {
    if (is_nan() || o.is_nan())
    {
      return dpp{nan{}};
    }
    else
    {
      dpp tmp(*this);

      tmp.v_.e += o.v_.e;

      auto r(tmp.v_.m * std::intmax_t(o.v_.m));

      while (std::abs(r) > pow<2>(M - 1) - 1)
      {
        r /= 10;
        ++tmp.v_.e;
      }

      tmp.v_.m = r;

      tmp.normalize();

      return tmp;
    }
  }

  constexpr auto& operator*=(dpp const& o) noexcept
  {
    if (is_nan() || o.is_nan())
    {
      *this = dpp{nan{}};
    }
    else
    {
      v_.e += o.v_.e;

      auto r(v_.m * std::intmax_t(o.v_.m));

      while (std::abs(r) > pow<2>(M - 1) - 1)
      {
        r /= 10;
        ++v_.e;
      }

      v_.m = r;

      normalize();
    }

    return *this;
  }

  //
  constexpr auto operator/(dpp const& o) const noexcept
  {
    if (is_nan() || o.is_nan() || !o.v_.m)
    {
      return dpp{nan{}};
    }
    else
    {
      dpp tmp(*this);

      tmp.v_.e -= o.v_.e;

      std::intmax_t r(tmp.v_.m);

      while (std::abs(r) < std::numeric_limits<std::intmax_t>::max() / 10)
      {
        r *= 10;
        --tmp.v_.e;
      }

      r /= o.v_.m;

      while (std::abs(r) > pow<2>(M - 1) - 1)
      {
        r /= 10;
        ++tmp.v_.e;
      }

      tmp.v_.m = r;

      tmp.normalize();

      return tmp;
    }
  }

  constexpr auto& operator/=(dpp const& o) noexcept
  {
    if (is_nan() || o.is_nan() || !o.v_.m)
    {
      *this = dpp{nan{}};
    }
    else
    {
      v_.e -= o.v_.e;

      std::intmax_t r(v_.m);

      while (std::abs(r) < std::numeric_limits<std::intmax_t>::max() / 10)
      {
        r *= 10;
        --v_.e;
      }

      r /= o.v_.m;

      while (std::abs(r) > pow<2>(M - 1) - 1)
      {
        r /= 10;
        ++v_.e;
      }

      v_.m = r;

      normalize();
    }

    return *this;
  }
};

using dec64 = dpp<56, 8>;
using dec32 = dpp<26, 6>;

//////////////////////////////////////////////////////////////////////////////
template <typename T, typename It>
constexpr T to_decimal(It i, It const end) noexcept
{
  if (i == end)
  {
    return {};
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
        return {};
    }

    int fcount{};

    typename T::value_type r{};

    for (; i != end; i = std::next(i))
    {
      switch (*i)
      {
        case '.':
          i = std::next(i);
          break;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          r = 10 * r + (*i - '0');
          continue;

        case '\0':
        {
          auto const tmp(T(r, -fcount));
          return positive ? tmp : -tmp;
        }

        default:
          return {};
      }

      break;
    }

    for (; i != end; ++fcount, i = std::next(i))
    {
      switch (*i)
      {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          r = 10 * r + (*i - '0');
          continue;

        case '\0':
          break;

        default:
          return {};
      }

      break;
    }

    auto const tmp(T(r, -fcount));
    return positive ? tmp : -tmp;
  }
}

template <typename T, typename S>
constexpr auto to_decimal(S const& s) noexcept ->
  decltype(std::cbegin(s), std::cend(s), T())
{
  return to_decimal<T>(std::cbegin(s), std::cend(s));
}

//////////////////////////////////////////////////////////////////////////////
template <unsigned M, unsigned E>
inline auto to_string(dpp<M, E> p)
{
  std::string r;

  if (p < 0)
  {
    p = -p;
    r.append(1, '-');
  }

  {
    typename dpp<M, E>::value_type i(p);
    r.append(std::to_string(i));

    p -= i;
  }

  if (p)
  {
    r.append(1, '.');

    while (p)
    {
      p *= 10;

      typename dpp<M, E>::value_type i(p);
      r.append(std::to_string(i));

      p -= i;
    }
  }

  return r;
}

template <unsigned M, unsigned E>
inline std::ostream& operator<<(std::ostream& os, dpp<M, E> p)
{
  if (std::ostream::sentry s(os); s)
  {
    if (p < 0)
    {
      p = -p;
      os << '-';
    }

    {
      typename dpp<M, E>::value_type i(p);
      os << i;

      p -= i;
    }

    if (p)
    {
      os << '.';

      while (p)
      {
        p *= 10;

        typename dpp<M, E>::value_type i(p);
        os << i;

        p -= i;
      }
    }
  }

  return os;
}

}

#endif // DPP_HPP
