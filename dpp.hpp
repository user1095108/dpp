#ifndef DPP_HPP
# define DPP_HPP
# pragma once

#include <climits>
#include <cmath> // std::pow
#include <cstdint>

#include <compare> // std::strong_ordering
#include <functional> // std::hash
#include <iterator> // std::begin(), std::end()
#include <optional> // std::optional
#include <ostream>
#include <string>
#include <tuple>
#include <type_traits>

#if (INTPTR_MAX >= INT64_MAX || defined(__EMSCRIPTEN__)) && !defined(_MSC_VER)
# define DPP_INT128T __int128
#else
# define DPP_INT128T void
#endif // DPP_INT128T

namespace dpp
{

using direct = struct {};
using nan = struct {};

using int_t = std::int32_t; // int type wide enough to deal with exponents

namespace detail
{

template <typename U>
constexpr static auto is_integral_v(
  std::is_integral_v<U> || std::is_same_v<U, DPP_INT128T>
);

template <typename U>
constexpr static auto is_signed_v(
  std::is_signed_v<U> || std::is_same_v<U, DPP_INT128T>
);

template <typename U>
consteval auto bit_size() noexcept
{
  return CHAR_BIT * sizeof(U);
}

template <typename U>
consteval auto min() noexcept
{
  if constexpr(is_signed_v<U>)
  {
    return U(U(1) << (bit_size<U>() - 1));
  }
  else
  {
    return U{};
  }
}

template <typename U>
consteval auto max() noexcept
{
  if constexpr(is_signed_v<U>)
  {
    return -U(min<U>() + U(1));
  }
  else
  {
    return ~U();
  }
}

template <auto min, auto max>
constexpr auto shift_left(auto m, auto& e, int_t i) noexcept
{
  if (m < 0)
  {
    for (; (m >= min / 10) && i--; m *= 10, --e);
  }
  else
  {
    for (; (m <= max / 10) && i--; m *= 10, --e);
  }

  return m;
}

// ae and be are minimal, cannot be reduced further, ae >= be, maximize be.
constexpr auto shift_right(auto m, int_t i) noexcept
{
  for (auto const c(selectsign<5>(m)); m && i--; m = (m + c) / 10);

  return m;
}


template <typename T, int B>
constexpr T pow(int_t e) noexcept
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

consteval int_t log10(auto const x, int_t const e = 0u) noexcept
{
  return pow<std::remove_cv_t<decltype(x)>, 10>(e) > x ? e : log10(x, e + 1);
}

template <auto a, typename B>
constexpr B selectsign(B const b) noexcept
{
  if constexpr(is_signed_v<decltype(a)> && is_signed_v<B>)
  {
    return b < 0 ? -a : a;
  }
  else
  {
    return a;
  }
}

}

template <unsigned M>
class dpp
{
public:
  using exp_type = std::int16_t;

  enum : exp_type
  {
    emin = detail::min<exp_type>(),
    emax = detail::max<exp_type>()
  };

  using mantissa_type = std::conditional_t<
    M == 16,
    std::int16_t,
    std::conditional_t<
      M == 32,
      std::int32_t,
      std::conditional_t<
        M == 64,
        std::int64_t,
        void
      >
    >
  >;

  enum : mantissa_type
  {
    mmin = detail::min<mantissa_type>(),
    mmax = detail::max<mantissa_type>()
  };

private:
  using doubled_t = std::conditional_t<
    std::is_same_v<mantissa_type, std::int16_t>,
    std::int32_t,
    std::conditional_t<
      std::is_same_v<mantissa_type, std::int32_t>,
      std::int64_t,
      std::conditional_t<
        std::is_same_v<mantissa_type, std::int64_t>,
        DPP_INT128T,
        void
      >
    >
  >;

  struct
  {
    mantissa_type m;
    exp_type e;
  } v_{};

public:
  constexpr dpp() = default;

  constexpr dpp(dpp const&) = default;
  constexpr dpp(dpp&&) = default;

  template <typename U>
  constexpr dpp(U m, int_t e) noexcept
    requires(detail::is_integral_v<U>)
  {
    enum : U
    {
      umin = detail::min<U>(),
      umax = detail::max<U>()
    };

    // slash m, if necessary
    if constexpr(detail::is_signed_v<U> && (detail::bit_size<U>() > M))
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

    if constexpr((detail::is_signed_v<U> && (detail::bit_size<U>() > M)) ||
      (std::is_unsigned_v<U> && (detail::bit_size<U>() >= M)))
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
    for (auto const c(detail::selectsign<5>(m)); (e <= emin) && m;
      m = (m + c) / 10, ++e);

    // normalize, minimize the exponent, if m non-zero
    if (m)
    {
      mantissa_type tm(m);

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
  constexpr dpp(U const m) noexcept
    requires(detail::is_integral_v<U>):
    dpp(std::conditional_t<
          detail::bit_size<U>() < detail::bit_size<mantissa_type>(),
          mantissa_type,
          U
        >(m),
        0
    )
  {
  }

  template <unsigned A>
  constexpr dpp(dpp<A> const o) noexcept:
    dpp(o.mantissa(), o.exponent())
  {
  }

  template <typename U>
  dpp(U f) noexcept
    requires(std::is_floating_point_v<U>)
  {
    if (std::isfinite(f))
    {
      int_t e{};

      // eliminate the fractional part, slash f, if necessary
      for (; std::trunc(f) != f; f *= U(10), --e);
      for (constexpr long double
        min(detail::min<std::intmax_t>()),
        max(detail::max<std::intmax_t>());
        (f < min) || (f > max); f /= 10, ++e);

      *this = {std::intmax_t(f), e};
    }
    else
    {
      *this = nan{};
    }
  }

  //
  constexpr dpp(mantissa_type const m, exp_type const e, direct) noexcept:
    v_{.m = m, .e = e}
  {
  }

  constexpr dpp(nan) noexcept: v_{.m = {}, .e = emin} { }

  //
  constexpr explicit operator bool() const noexcept
  {
    return v_.m || isnan(*this);
  }

  template <typename T>
  explicit (sizeof(T) != sizeof(v_.m)) operator T() const noexcept
    requires(std::is_floating_point_v<T>)
  {
    if (isnan(*this))
    {
      return NAN;
    }
    else if (auto m(v_.m); m)
    {
      int_t e(v_.e);

      for (; !(m % 10); m /= 10, ++e);

      return m * std::pow(T(10), e);
    }
    else
    {
      return T{};
    }
  }

  // this function is unsafe, take a look at to_integral() for safety
  template <typename T>
  constexpr explicit operator T() const noexcept
    requires(detail::is_integral_v<T>)
  {
    if (int_t e(v_.e); e > 0)
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
  constexpr dpp& operator=(dpp const&) = default;
  constexpr dpp& operator=(dpp&&) = default;

  #define DPP_ASSIGNMENT(OP)\
    template <typename U>\
    constexpr auto& operator OP ## =(U&& a) noexcept\
    {\
      return *this = *this OP std::forward<U>(a);\
    }

  DPP_ASSIGNMENT(+)
  DPP_ASSIGNMENT(-)
  DPP_ASSIGNMENT(*)
  DPP_ASSIGNMENT(/)

  // increment, decrement
  constexpr auto& operator++() noexcept { return *this += 1; }
  constexpr auto& operator--() noexcept { return *this -= 1; }

  constexpr auto operator++(int) const noexcept { return *this + 1; }
  constexpr auto operator--(int) const noexcept { return *this - 1; }

  // arithmetic
  constexpr auto operator+() const noexcept { return *this; }

  constexpr auto operator-() const noexcept
  {
    // we need to do it like this, as negating the mantissa can overflow
    return isnan(*this) ?
      dpp{nan{}} :
      mmin == v_.m ? dpp(-doubled_t(mmin), v_.e) : dpp(-v_.m, v_.e, direct{});
  }

  constexpr dpp operator+(dpp const o) const noexcept
  {
    enum : doubled_t
    {
      rmin = detail::min<doubled_t>() / 2,
      rmax = detail::max<doubled_t>() / 2
    };

    if (isnan(*this) || isnan(o))
    {
      return nan{};
    }
    else
    {
      if (doubled_t mb(o.v_.m); !mb)
      {
        return *this;
      }
      else if (doubled_t ma(v_.m); !ma)
      {
        return o;
      }
      else
      {
        if (int_t ea(v_.e), eb(o.v_.e); ea < eb)
        {
          return {
            detail::shift_left<rmin, rmax>(mb, eb, eb - ea) +
            detail::shift_right(ma, eb - ea),
            eb
          };
        }
        else
        {
          return {
            detail::shift_left<rmin, rmax>(ma, ea, ea - eb) +
            detail::shift_right(mb, ea - eb),
            ea
          };
        }
      }
    }
  }

  constexpr dpp operator-(dpp const o) const noexcept
  {
    enum : doubled_t
    {
      rmin = detail::min<doubled_t>() / 2,
      rmax = detail::max<doubled_t>() / 2
    };

    if (isnan(*this) || isnan(o))
    {
      return nan{};
    }
    else
    {
      int_t eb(o.v_.e);

      if (doubled_t mb(o.v_.m); !mb)
      {
        return *this;
      }
      else if (doubled_t ma(v_.m); !ma)
      {
        return mmin == mb ? dpp(-mb, eb) : dpp(-mb, eb, direct{});
      }
      else
      {
        if (int_t ea(v_.e); ea < eb)
        {
          return {
            detail::shift_left<rmin, rmax>(-mb, eb, eb - ea) +
            detail::shift_right(ma, eb - ea),
            eb
          };
        }
        else
        {
          return {
            detail::shift_left<rmin, rmax>(ma, ea, ea - eb) -
            detail::shift_right(mb, ea - eb),
            ea
          };
        }
      }
    }
  }

  constexpr dpp operator*(dpp const o) const noexcept
  {
    return isnan(*this) || isnan(o) ?
      nan{} :
      dpp<M>{doubled_t(v_.m) * o.v_.m, int_t(v_.e) + o.v_.e};
  }

  constexpr dpp operator/(dpp const o) const noexcept
  {
    enum : doubled_t
    {
      rmin = detail::min<doubled_t>(),
      rmax = detail::max<doubled_t>()
    };

    // dp is the exponent, that generates the maximal power of 10,
    // that fits into doubled_t
    // 10^dp > rmax, hence 10^(dp - 1) <= rmax
    enum : int_t { dp = detail::log10((long double)(rmax)) - 1 };

    static_assert(detail::pow<doubled_t, 10>(dp) <= rmax - 5);
    static_assert(-detail::pow<doubled_t, 10>(dp) >= rmin + 5);

    if (auto const om(o.v_.m); isnan(*this) || isnan(o) || !om) // div by 0
    {
      return nan{};
    }
    else if (v_.m) // div 0
    {
      doubled_t const m(v_.m);

      int_t e(-dp + v_.e - o.v_.e);

      // we want an approximation to m * (10^dp / om)
      auto q(detail::pow<doubled_t, 10>(dp) / om);

      // fit m * q into doubled_t, m * q <= rmax, m * q >= rmin
      if (auto const am(m < 0 ? -m : m); q < 0)
      {
        for (auto const a(rmin / am); q < a; q = (q - 5) / 10, ++e);
      }
      else
      {
        for (auto const a(rmax / am); q > a; q = (q + 5) / 10, ++e);
      }

      return {m * q, e};
    }
    else
    {
      return {};
    }
  }

  //
  constexpr bool operator==(dpp const o) const noexcept
  {
    return !isnan(*this) && !isnan(o) && (v_.m == o.v_.m) && (v_.e == o.v_.e);
  }

  constexpr bool operator<(dpp const o) const noexcept
  {
    enum : doubled_t
    {
      rmin = detail::min<doubled_t>() / 2,
      rmax = detail::max<doubled_t>() / 2
    };

    if (isnan(*this) || isnan(o))
    {
      return false;
    }
    else if (doubled_t ma(v_.m), mb(o.v_.m); ma && mb)
    {
      if (int_t ea(v_.e), eb(o.v_.e); ea < eb)
      {
        return detail::shift_left<rmin, rmax>(mb, eb, eb - ea) >
          detail::shift_right(ma, eb - ea);
      }
      else
      {
        return detail::shift_left<rmin, rmax>(ma, ea, ea - eb) <
          detail::shift_right(mb, ea - eb);
      }
    }
    else
    {
      return ma < mb;
    }
  }

  //
  static constexpr dpp min() noexcept { return {mmin, emax}; }
  static constexpr dpp max() noexcept { return {mmax, emax}; }

  //
  constexpr auto exponent() const noexcept { return v_.e; }
  constexpr auto mantissa() const noexcept { return v_.m; }

  //
  constexpr auto packed() const noexcept { return std::tuple(v_.m, v_.e); }

  static constexpr dpp unpack(auto const& o) noexcept
  {
    return {std::get<0>(o), std::get<1>(o)};
  }
};

using d64 = dpp<64>;
using d32 = dpp<32>;
using d16 = dpp<16>;

// type promotions
#define DPP_TYPE_PROMOTION(OP)\
template <unsigned A, unsigned B>\
constexpr auto operator OP (dpp<A> const a, dpp<B> const b) noexcept\
{\
  if constexpr (A < B)\
    return dpp<B>(a) OP b;\
  else\
    return a OP dpp<A>(b);\
}

DPP_TYPE_PROMOTION(+)
DPP_TYPE_PROMOTION(-)
DPP_TYPE_PROMOTION(*)
DPP_TYPE_PROMOTION(/)
DPP_TYPE_PROMOTION(==)
DPP_TYPE_PROMOTION(<)

// comparison operators
template <unsigned A, unsigned B>
constexpr auto operator!=(dpp<A> const a, dpp<B> const b) noexcept
{
  return !(a == b);
}

template <unsigned A, unsigned B>
constexpr auto operator<=(dpp<A> const a, dpp<B> const b) noexcept
{
  return !(b < a);
}

template <unsigned A, unsigned B>
constexpr auto operator>(dpp<A> const a, dpp<B> const b) noexcept
{
  return b < a;
}

template <unsigned A, unsigned B>
constexpr auto operator>=(dpp<A> const a, dpp<B> const b) noexcept
{
  return !(a < b);
}

template <unsigned A, unsigned B>
constexpr auto operator<=>(dpp<A> const a, dpp<B> const b) noexcept
{
  return a == b ?
    std::strong_ordering::equal :
    a < b ? std::strong_ordering::less : std::strong_ordering::greater;
}

// conversions
#define DPP_LEFT_CONVERSION(OP)\
template <unsigned A, typename U>\
constexpr auto operator OP (U&& a, dpp<A> const b) noexcept\
  requires(std::is_arithmetic_v<std::remove_cvref_t<U>>)\
{\
  return dpp<A>(std::forward<U>(a)) OP b;\
}

DPP_LEFT_CONVERSION(+)
DPP_LEFT_CONVERSION(-)
DPP_LEFT_CONVERSION(*)
DPP_LEFT_CONVERSION(/)
DPP_LEFT_CONVERSION(==)
DPP_LEFT_CONVERSION(!=)
DPP_LEFT_CONVERSION(<)
DPP_LEFT_CONVERSION(<=)
DPP_LEFT_CONVERSION(>)
DPP_LEFT_CONVERSION(>=)
DPP_LEFT_CONVERSION(<=>)

#define DPP_RIGHT_CONVERSION(OP)\
template <unsigned A, typename U>\
constexpr auto operator OP (dpp<A> const a, U&& b) noexcept\
  requires(std::is_arithmetic_v<std::remove_cvref_t<U>>)\
{\
  return a OP dpp<A>(std::forward<U>(b));\
}

DPP_RIGHT_CONVERSION(+)
DPP_RIGHT_CONVERSION(-)
DPP_RIGHT_CONVERSION(*)
DPP_RIGHT_CONVERSION(/)
DPP_RIGHT_CONVERSION(==)
DPP_RIGHT_CONVERSION(!=)
DPP_RIGHT_CONVERSION(<)
DPP_RIGHT_CONVERSION(<=)
DPP_RIGHT_CONVERSION(>)
DPP_RIGHT_CONVERSION(>=)
DPP_RIGHT_CONVERSION(<=>)

// misc
template <unsigned M>
constexpr auto isfinite(dpp<M> const a) noexcept
{
  return !isnan(a);
}

template <unsigned M>
constexpr auto isinf(dpp<M> const a) noexcept
{
  return isnan(a);
}

template <unsigned M>
constexpr auto isnan(dpp<M> const a) noexcept
{
  return dpp<M>::emin == a.exponent();
}

template <unsigned M>
constexpr auto isnormal(dpp<M> const a) noexcept
{
  return !isnan(a);
}

//
template <unsigned M>
constexpr auto trunc(dpp<M> const a) noexcept
{
  if (int_t e(a.exponent()); !isnan(a) && (e < 0))
  {
    auto m(a.mantissa());

    for (; m && e++; m /= 10);

    return dpp<M>(m, 0);
  }
  else
  {
    return a;
  }
}

template <unsigned M>
constexpr auto ceil(dpp<M> const a) noexcept
{
  auto const t(trunc(a));

  return t + (t < a);
}

template <unsigned M>
constexpr auto floor(dpp<M> const a) noexcept
{
  auto const t(trunc(a));

  return t - (t > a);
}

template <unsigned M>
constexpr auto round(dpp<M> const a) noexcept
{
  constexpr dpp<M> c(5, -1);

  return a.exponent() < 0 ?
    trunc(a.mantissa() < 0 ? a - c : a + c) :
    a;
}

template <unsigned M>
constexpr auto abs(dpp<M> const a) noexcept
{
  return a.mantissa() < 0 ? -a : a;
}

// conversions
template <typename T>
constexpr T to_decimal(std::input_iterator auto i,
  decltype(i) const end) noexcept
{
  if (i == end)
  {
    return nan{};
  }
  else
  {
    enum : std::intmax_t
    {
      rmax = detail::max<std::intmax_t>(),
      rmin = detail::min<std::intmax_t>()
    };

    bool positive{true};

    switch (*i)
    {
      case '-':
        positive = false;
        [[fallthrough]];

      case '+':
        i = std::next(i);
        break;

      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
      case '.':
        break;

      default:
        return nan{};
    }

    std::intmax_t r{};
    int_t e{};

    auto const scandigit([&](decltype(r) const d) noexcept
      {
        if (positive)
        {
          if (r <= rmax / 10)
          {
            if (auto const t(10 * r); t <= rmax - d)
            {
              r = t + d;

              return false;
            }
          }
        }
        else if (r >= rmin / 10)
        {
          if (auto const t(10 * r); t >= rmin + d)
          {
            r = t - d;

            return false;
          }
        }

        return true;
      }
    );

    for (; end != i; i = std::next(i))
    {
      switch (*i)
      {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          if (scandigit(*i - '0')) break; else continue;

        case '.':
          i = std::next(i);
          break;

        case '\0':
          return {r, 0};

        default:
          return nan{};
      }

      break;
    }

    for (; end != i; i = std::next(i))
    {
      switch (*i)
      {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          if (scandigit(*i - '0')) break; else {--e; continue;}

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

template <typename T = std::intmax_t, unsigned M>
constexpr std::optional<T> to_integral(dpp<M> const p) noexcept
{
  enum : T
  {
    tmin = detail::min<T>(),
    tmax = detail::max<T>()
  };

  if (isnan(p))
  {
    return {};
  }
  else
  {
    T m(p.mantissa());

    if (int_t e(p.exponent()); e <= 0)
    {
      for (; m && e++; m /= 10);

      return m;
    }
    else
    {
      do
      {
        if ((m >= tmin / 10) && (m <= tmax / 10))
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

template <unsigned M>
std::string to_string(dpp<M> p)
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
      int_t e(t.exponent());

      for (; !(m % 10); m /= 10, ++e);

      r.append(std::to_string(m)).append(e, '0');
    }
    else
    {
      p.mantissa() < 0 ? r.append("-0", 2) : r.append(1, '0');
    }

    auto m(p.mantissa());

    if (int_t e(p.exponent()); (e < 0) && m)
    {
      for (; !(m % 10); m /= 10, ++e);

      auto const tmp(std::to_string(std::abs(m)));

      r.append(1, '.').append(-e - tmp.size(), '0').append(tmp);
    }

    return r;
  }
}

template <unsigned M>
inline auto& operator<<(std::ostream& os, dpp<M> const p)
{
  return os << to_string(p);
}

//////////////////////////////////////////////////////////////////////////////
namespace literals
{

#define DPP_LITERAL(ID)\
template <char ...c>\
constexpr auto operator "" _d ## ID() noexcept\
{\
  return to_decimal<d ## ID>((char const[sizeof...(c)]){c...});\
}

DPP_LITERAL(16)
DPP_LITERAL(32)
DPP_LITERAL(64)

}

}

//////////////////////////////////////////////////////////////////////////////
namespace std
{

template <unsigned M>
struct hash<dpp::dpp<M>>
{
  constexpr auto operator()(dpp::dpp<M> const a) const noexcept
  {
    auto const hash_combine(
      [](auto&& ...v) noexcept requires(bool(sizeof...(v)))
      {
        std::size_t seed{672807365};

        return (
          (
            seed ^= std::hash<std::remove_cvref_t<decltype(v)>>()(
              std::forward<decltype(v)>(v)) + 0x9e3779b9 + (seed << 6) +
              (seed >> 2)
          ),
          ...
        );
      }
    );

    return hash_combine(a.mantissa(), a.exponent());
  }
};

}

#endif // DPP_HPP
