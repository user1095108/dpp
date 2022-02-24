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
#include <type_traits>
#include <utility>

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
constexpr static auto bit_size_v(CHAR_BIT * sizeof(U));

template <typename U>
constexpr static U min_v(is_signed_v<U> ? U(1) << (bit_size_v<U> - 1) : U{});

template <typename U>
constexpr static U max_v(is_signed_v<U> ? -(min_v<U> + U(1)) : ~U());

template <auto a, typename B>
constexpr B selectsign(B const b) noexcept
{
  static_assert(a > 0);

  if constexpr(is_signed_v<decltype(a)> && is_signed_v<B>)
  {
    return b < 0 ? -a : a;
  }
  else
  {
    return a;
  }
}

constexpr auto shift_left(auto m, auto& e, auto i) noexcept
{ // we need to be mindful of overflow, since we are shifting left
  if (m < 0)
  {
    for (; (m >= min_v<decltype(m)> / 20) && i; --i, m *= 10, --e);
  }
  else
  {
    for (; (m <= max_v<decltype(m)> / 20) && i; --i, m *= 10, --e);
  }

  return m;
}

constexpr auto shift_right(auto m, auto i) noexcept
{
  for (auto const c(selectsign<5>(m)); m && i; --i, m = (m + c) / 10);

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

}

template <unsigned M>
class dpp
{
public:
  using exp_type = std::int16_t;

  enum : exp_type
  {
    emin = detail::min_v<exp_type>,
    emax = detail::max_v<exp_type>
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
    mmin = detail::min_v<mantissa_type>,
    mmax = detail::max_v<mantissa_type>
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
  } v_;

public:
  dpp() = default;

  constexpr dpp(dpp const&) = default;
  constexpr dpp(dpp&&) = default;

  template <typename U>
  constexpr dpp(U m, int_t e) noexcept
    requires(detail::is_integral_v<U>)
  {
    enum : U
    {
      umin = detail::min_v<U>,
      umax = detail::max_v<U>
    };

    // reduction of the exponent is a side effect of +- ops
    if (e > emax)
    {
      *this = nan{};
    }
    else
    {
      // slash m, if necessary
      if constexpr(detail::is_signed_v<U> && (detail::bit_size_v<U> > M))
      {
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
      }

      if constexpr((detail::is_signed_v<U> && (detail::bit_size_v<U> > M)) ||
        (std::is_unsigned_v<U> && (detail::bit_size_v<U> >= M)))
      {
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
      }

      // additional slashing, if necessary
      for (auto const c(detail::selectsign<5>(m)); (e <= emin) && m;
        m = (m + c) / 10, ++e);

      v_.m = m;
      v_.e = e;
    }
  }

  template <typename U>
  constexpr dpp(U const m) noexcept
    requires(detail::is_integral_v<U>):
    dpp(std::conditional_t<
          detail::bit_size_v<U> < detail::bit_size_v<mantissa_type>,
          mantissa_type,
          U
        >(m),
        0
    )
  {
  }

  template <unsigned A>
  constexpr dpp(dpp<A> const& o) noexcept:
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
        min(detail::min_v<std::intmax_t>),
        max(detail::max_v<std::intmax_t>);
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
      return {};
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

  constexpr dpp operator+(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o))
    {
      return nan{};
    }
    else if (auto const m(v_.m); !m)
    {
      return o;
    }
    else if (auto const om(o.v_.m); !om)
    {
      return *this;
    }
    else
    {
      doubled_t const ma(m), mb(om);
      int_t ea(v_.e), eb(o.v_.e);

      return ea < eb ?
        dpp{
          detail::shift_left(mb, eb, eb - ea) + // reduce eb
          detail::shift_right(ma, eb - ea), // increase ea
          eb
        } :
        dpp{
          detail::shift_left(ma, ea, ea - eb) + // increase ea
          detail::shift_right(mb, ea - eb), // reduce eb
          ea
        };
    }
  }

  constexpr dpp operator-(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o))
    {
      return nan{};
    }
    else if (auto const m(v_.m), om(o.v_.m); !om)
    {
      return *this;
    }
    else if (auto const oe(o.v_.e); !m)
    {
      return mmin == om ?
        dpp(-doubled_t(mmin), oe) :
        dpp(-om, oe, direct{});
    }
    else
    {
      doubled_t const ma(m), mb(-om);
      int_t ea(v_.e), eb(oe);

      return ea < eb ?
        dpp{
          detail::shift_left(mb, eb, eb - ea) +
          detail::shift_right(ma, eb - ea),
          eb
        } :
        dpp{
          detail::shift_left(ma, ea, ea - eb) +
          detail::shift_right(mb, ea - eb),
          ea
        };
    }
  }

  constexpr dpp operator*(dpp const& o) const noexcept
  {
    return isnan(*this) || isnan(o) ?
      nan{} :
      dpp{doubled_t(v_.m) * o.v_.m, int_t(v_.e) + o.v_.e};
  }

  constexpr dpp operator/(dpp const& o) const noexcept
  {
    enum : doubled_t
    {
      min = detail::min_v<doubled_t>,
      max = detail::max_v<doubled_t>
    };

    // dp is the exponent, that generates the maximal power of 10,
    // that fits into doubled_t
    // 10^dp > max, hence 10^(dp - 1) <= rmax
    enum : int_t { dp = detail::log10((long double)(max)) - 1 };

    static_assert(detail::pow<doubled_t, 10>(dp) <= max - 5);
    static_assert(-detail::pow<doubled_t, 10>(dp) >= min + 5);

    if (auto const om(o.v_.m); isnan(*this) || isnan(o) || !om) // div by 0
    {
      return nan{};
    }
    else if (v_.m) // div by 0
    {
      doubled_t const m(v_.m);
      int_t e(-dp + v_.e - o.v_.e);

      // we want an approximation to m * (10^dp / om)
      auto q(detail::pow<doubled_t, 10>(dp) / om);

      // fit m * q into doubled_t, m * q <= max, m * q >= min
      if (auto const am(m < 0 ? -m : m); q < 0)
      {
        for (auto const a(min / am); q < a; q = (q - 5) / 10, ++e);
      }
      else
      {
        for (auto const a(max / am); q > a; q = (q + 5) / 10, ++e);
      }

      return {m * q, e};
    }
    else
    {
      return {};
    }
  }

  //
  constexpr bool operator==(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o))
    {
      return false;
    }
    else if (auto const m(v_.m), om(o.v_.m); !m || !om)
    {
      return m == om;
    }
    else
    {
      doubled_t const ma(m), mb(om);
      int_t ea(v_.e), eb(o.v_.e);

      return ea < eb ?
        detail::shift_left(mb, eb, eb - ea) ==
        detail::shift_right(ma, eb - ea) :
        detail::shift_left(ma, ea, ea - eb) ==
        detail::shift_right(mb, ea - eb);
    }
  }

  constexpr bool operator<(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o))
    {
      return false;
    }
    else if (auto const m(v_.m), om(o.v_.m); !m || !om)
    {
      return m < om;
    }
    else
    {
      doubled_t const ma(m), mb(om);
      int_t ea(v_.e), eb(o.v_.e);

      return ea < eb ?
        detail::shift_left(mb, eb, eb - ea) >
        detail::shift_right(ma, eb - ea) :
        detail::shift_left(ma, ea, ea - eb) <
        detail::shift_right(mb, ea - eb);
    }
  }

  //
  static constexpr dpp min() noexcept { return {mmin, emax, direct{}}; }
  static constexpr dpp max() noexcept { return {mmax, emax, direct{}}; }

  //
  constexpr auto exponent() const noexcept { return v_.e; }
  constexpr auto mantissa() const noexcept { return v_.m; }

  //
  constexpr auto packed() const noexcept { return std::pair(v_.m, v_.e); }

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
constexpr auto operator OP (dpp<A> const& a, dpp<B> const& b) noexcept\
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
constexpr auto operator!=(dpp<A> const& a, dpp<B> const& b) noexcept
{
  return !(a == b);
}

template <unsigned A, unsigned B>
constexpr auto operator<=(dpp<A> const& a, dpp<B> const& b) noexcept
{
  return !(b < a);
}

template <unsigned A, unsigned B>
constexpr auto operator>(dpp<A> const& a, dpp<B> const& b) noexcept
{
  return b < a;
}

template <unsigned A, unsigned B>
constexpr auto operator>=(dpp<A> const& a, dpp<B> const& b) noexcept
{
  return !(a < b);
}

template <unsigned A, unsigned B>
constexpr auto operator<=>(dpp<A> const& a, dpp<B> const& b) noexcept
{
  if (isnan(a) || isnan(b))
  {
    return std::partial_ordering::unordered;
  }
  else
  {
    auto const am(a.mantissa());
    auto const bm(b.mantissa());

    if (!am || !bm)
    {
      return std::partial_ordering(am <=> bm);
    }
    else
    {
      using greater_t = std::conditional_t<A < B, dpp<B>, dpp<A>>;

      typename greater_t::doubled_t ma(am), mb(bm);

      if (typename greater_t::int_t ea(a.exponent()), eb(b.exponent());
        ea < eb)
      {
        mb = detail::shift_left(mb, eb, eb - ea);
        ma = detail::shift_right(ma, eb - ea);
      }
      else
      {
        ma = detail::shift_left(ma, ea, ea - eb);
        mb = detail::shift_right(mb, ea - eb);
      }

      return std::partial_ordering(ma <=> mb);
    }
  }
}

// conversions
#define DPP_LEFT_CONVERSION(OP)\
template <unsigned A, typename U>\
constexpr auto operator OP (U&& a, dpp<A> const& b) noexcept\
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
constexpr auto operator OP (dpp<A> const& a, U&& b) noexcept\
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
constexpr auto isfinite(dpp<M> const& a) noexcept
{
  return !isnan(a);
}

template <unsigned M>
constexpr auto isinf(dpp<M> const& a) noexcept
{
  return isnan(a);
}

template <unsigned M>
constexpr auto isnan(dpp<M> const& a) noexcept
{
  return dpp<M>::emin == a.exponent();
}

template <unsigned M>
constexpr auto isnormal(dpp<M> const& a) noexcept
{
  return !isnan(a);
}

//
template <unsigned M>
constexpr auto trunc(dpp<M> const& a) noexcept
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
constexpr auto ceil(dpp<M> const& a) noexcept
{
  auto const t(trunc(a));

  return t + (t < a);
}

template <unsigned M>
constexpr auto floor(dpp<M> const& a) noexcept
{
  auto const t(trunc(a));

  return t - (t > a);
}

template <unsigned M>
constexpr auto round(dpp<M> const& a) noexcept
{
  constexpr dpp<M> c(5, -1);

  return a.exponent() < 0 ?
    trunc(a.mantissa() < 0 ? a - c : a + c) :
    a;
}

template <unsigned M>
constexpr auto abs(dpp<M> const& a) noexcept
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
      min = detail::min_v<std::intmax_t>,
      max = detail::max_v<std::intmax_t>
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
          if (r <= max / 10)
          {
            if (auto const t(10 * r); t <= max - d)
            {
              r = t + d;

              return false;
            }
          }
        }
        else if (r >= min / 10)
        {
          if (auto const t(10 * r); t >= min + d)
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
    min = detail::min_v<T>,
    max = detail::max_v<T>
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
        if ((m >= min / 10) && (m <= max / 10))
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
  auto operator()(dpp::dpp<M> const& a) const noexcept
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

    if (auto m(a.mantissa()); m)
    {
      dpp::int_t e(a.exponent());

      for (; !(m % 10); m /= 10, ++e); // slash zeros

      return hash_combine(m, e);
    }
    else
    {
      return hash_combine(decltype(m){}, dpp::int_t{});
    }
  }
};

}

#endif // DPP_HPP
