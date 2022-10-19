#ifndef DPP_HPP
# define DPP_HPP
# pragma once

#include <climits>
#include <cmath> // std::pow
#include <cstdint>

#include <concepts> // std::floating_point, std::integral
#include <compare> // std::partial_ordering
#include <functional> // std::hash
#include <iterator> // std::begin(), std::end()
#include <optional> // std::optional
#include <ostream>
#include <string>
#include <type_traits>
#include <utility> // std::forward()

#if (INTPTR_MAX >= INT64_MAX || defined(__EMSCRIPTEN__)) && !defined(_MSC_VER)
# define DPP_INT128T __int128
#else
# define DPP_INT128T void
#endif // DPP_INT128T

namespace dpp
{

struct direct{};
struct nan{};

using int_t = std::int32_t; // int type wide enough to deal with exponents

namespace detail
{

template <typename U>
concept arithmetic =
  std::is_arithmetic_v<U> || std::is_same_v<std::remove_cv_t<U>, DPP_INT128T>;

template <typename U>
concept integral =
  std::integral<U> || std::is_same_v<std::remove_cv_t<U>, DPP_INT128T>;

template <typename U>
static constexpr auto is_signed_v(
  std::is_signed_v<U> || std::is_same_v<std::remove_cv_t<U>, DPP_INT128T>
);

template <typename U>
static constexpr auto bit_size_v(CHAR_BIT * sizeof(U));

template <typename U>
static constexpr U min_v(is_signed_v<U> ? U(1) << (bit_size_v<U> - 1) : U{});

template <typename U>
static constexpr U max_v(is_signed_v<U> ? -(min_v<U> + U(1)) : ~U());

constexpr auto hash_combine(auto&& ...v) noexcept requires(bool(sizeof...(v)))
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

constexpr void shift_left(auto& m, int_t& e, int_t i) noexcept
{ // we need to be mindful of overflow, since we are shifting left
  if (m < 0)
  {
    for (; (m >= min_v<std::remove_cvref_t<decltype(m)>> / 20) && i;
      --i, m *= 10, --e);
  }
  else
  {
    for (; (m <= max_v<std::remove_cvref_t<decltype(m)>> / 20) && i;
      --i, m *= 10, --e);
  }
}

constexpr void shift_right(auto& m, int_t i) noexcept
{
  for (; m && i; --i, m /= 10);
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

consteval int_t log10(auto const x, int_t const e = {}) noexcept
{
  return pow<std::remove_cv_t<decltype(x)>, 10>(e) > x ? e : log10(x, e + 1);
}

constexpr auto trunc(auto&& f) noexcept
{
#if defined(__DJGPP__)
  if constexpr(std::is_same_v<long double, std::remove_cvref_t<decltype(f)>>)
    return ::truncl(std::forward<decltype(f)>(f));
  else if constexpr(std::is_same_v<double, std::remove_cvref_t<decltype(f)>>)
    return ::trunc(std::forward<decltype(f)>(f));
  else if constexpr(std::is_same_v<float, std::remove_cvref_t<decltype(f)>>)
    return ::truncf(std::forward<decltype(f)>(f));
#else
  return std::trunc(std::forward<decltype(f)>(f));
#endif // __DJGPP__
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

public:
  struct
  {
    mantissa_type m;
    exp_type e;
  } v_;

public:
  dpp() = default;

  dpp(dpp const&) = default;
  dpp(dpp&&) = default;

  template <detail::integral U>
  constexpr dpp(U m, int_t e) noexcept
  {
    // reduction of the exponent is a side effect of +- ops
    if (e > emax)
    {
      *this = nan{};
    }
    else
    {
      if constexpr(detail::is_signed_v<U> && (detail::bit_size_v<U> > M))
      {
        if (m < mmin)
        {
          for (++e; m < 10 * U(mmin) + 5; m /= 10, ++e);

          m = (m - 5) / 10;
        }
        else if (m > mmax)
        {
          for (++e; m > 10 * U(mmax) - 5; m /= 10, ++e);

          m = (m + 5) / 10;
        }
      }
      else if constexpr(std::is_unsigned_v<U> && (detail::bit_size_v<U> >= M))
      {
        if (m > mmax)
        {
          for (++e; m > 10 * U(mmax) - 5; m /= 10, ++e);

          m = (m + 5) / 10;
        }
      }

      //
      for (; e <= emin; ++e, m /= 10);

      //
      v_.m = m;
      v_.e = e;
    }
  }

  constexpr dpp(detail::integral auto const m) noexcept: dpp(m, {}) { }

  template <unsigned A>
  constexpr dpp(dpp<A> const& o) noexcept:
    dpp(o.mantissa(), o.exponent())
  {
  }

  dpp(std::floating_point auto f) noexcept
  {
    if (std::isfinite(f))
    {
      int_t e{};

      {
        decltype(f) const k(10);

        // eliminate the fractional part, slash f, if necessary
        for (; detail::trunc(f) != f; f *= k, --e);
        for (long double const
          min(detail::min_v<std::intmax_t>),
          max(detail::max_v<std::intmax_t>);
          (f < min) || (f > max); f /= k, ++e);
      }

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

  template <std::floating_point T>
  explicit (sizeof(T) != sizeof(v_.m)) operator T() const noexcept
  {
    if (isnan(*this))
    {
      return NAN;
    }
    else if (auto m(v_.m); m)
    {
      int_t e(v_.e);

      for (; !(m % 10); m /= 10, ++e);

      return m * std::pow(T{10}, e);
    }
    else
    {
      return {};
    }
  }

  // this function is unsafe, take a look at to_integral() for safety
  template <detail::integral T>
  constexpr explicit operator T() const noexcept
  {
    if (auto e(v_.e); e < 0)
    {
      auto m(v_.m);

      for (; m && e; ++e, m /= T{10});

      return m;
    }
    else
    {
      return v_.m * detail::pow<T, 10>(e);
    }
  }

  // assignment
  dpp& operator=(dpp const&) = default;
  dpp& operator=(dpp&&) = default;

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
  constexpr auto& operator++() noexcept
  {
    return *this += dpp{mantissa_type{1}, {}, direct{}};
  }

  constexpr auto& operator--() noexcept
  {
    return *this -= dpp{mantissa_type{1}, {}, direct{}};
  }

  constexpr auto operator++(int) noexcept
  {
    auto const r(*this); ++*this; return r;
  }

  constexpr auto operator--(int) noexcept
  {
    auto const r(*this); --*this; return r;
  }

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
      doubled_t ma(m), mb(om);

      if (int_t ea(v_.e), eb(o.v_.e); ea < eb)
      {
        detail::shift_left(mb, eb, eb - ea); // reduce eb
        detail::shift_right(ma, eb - ea); // increase ea

        return {ma + mb, eb};
      }
      else
      {
        detail::shift_left(ma, ea, ea - eb);
        detail::shift_right(mb, ea - eb);

        return {ma + mb, ea};
      }
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
    { // prevent overflow
      return mmin == om ? dpp(-doubled_t(mmin), oe) : dpp(-om, oe, direct{});
    }
    else
    {
      doubled_t ma(m), mb(om);

      if (int_t ea(v_.e), eb(oe); ea < eb)
      {
        detail::shift_left(mb, eb, eb - ea); // reduce eb
        detail::shift_right(ma, eb - ea); // increase ea

        return {ma - mb, eb};
      }
      else
      {
        detail::shift_left(ma, ea, ea - eb);
        detail::shift_right(mb, ea - eb);

        return {ma - mb, ea};
      }
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
    enum : int_t
    {
      dp = detail::log10((long double)(detail::max_v<doubled_t>)) - 1
    };

    if (auto const om(o.v_.m); !om || isnan(*this) || isnan(o))
    {
      return nan{};
    }
    else
    {
      auto e(-dp + v_.e - o.v_.e);
      auto m(detail::pow<doubled_t, 10>(dp) / o.v_.m);

      if (m < mmin)
      {
        for (++e; m < 10 * doubled_t(mmin) + 5; m /= 10, ++e);

        m = (m - 5) / 10;
      }
      else if (m > mmax)
      {
        for (++e; m > 10 * doubled_t(mmax) - 5; m /= 10, ++e);

        m = (m + 5) / 10;
      }

      return {m * v_.m, e};
    }
  }

  //
  constexpr std::partial_ordering operator<=>(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o))
    {
      return std::partial_ordering::unordered;
    }
    else if (auto const m(v_.m), om(o.v_.m); !m || !om)
    {
      return m <=> om;
    }
    else
    {
      doubled_t ma(m), mb(om);

      {
        int_t ea(v_.e), eb(o.v_.e); // important to prevent overflow

        ea < eb ?
          detail::shift_left(mb, eb, eb - ea),
          detail::shift_right(ma, eb - ea) :
          (detail::shift_left(ma, ea, ea - eb),
          detail::shift_right(mb, ea - eb));
      }

      return ma <=> mb;
    }
  }

  //
  static constexpr dpp min() noexcept { return {mmin, emax, direct{}}; }
  static constexpr dpp max() noexcept { return {mmax, emax, direct{}}; }

  //
  constexpr auto exponent() const noexcept { return v_.e; }
  constexpr auto mantissa() const noexcept { return v_.m; }
};

using d64 = dpp<64>;
using d32 = dpp<32>;
using d16 = dpp<16>;

// type promotions
#define DPP_TYPE_PROMOTION(OP)\
template <unsigned A, unsigned B>\
constexpr auto operator OP (dpp<A> const& a, dpp<B> const& b) noexcept\
{\
  if constexpr(A < B)\
    return dpp<B>(a) OP b;\
  else\
    return a OP dpp<A>(b);\
}

DPP_TYPE_PROMOTION(+)
DPP_TYPE_PROMOTION(-)
DPP_TYPE_PROMOTION(*)
DPP_TYPE_PROMOTION(/)
DPP_TYPE_PROMOTION(<=>)

template <unsigned A, unsigned B>
constexpr bool operator==(dpp<A> const& a, dpp<B> const& b) noexcept
{
  return a <=> b == 0;
}

// conversions
#define DPP_LEFT_CONVERSION(OP)\
template <unsigned A>\
constexpr auto operator OP (detail::arithmetic auto const a,\
  dpp<A> const& b) noexcept\
{\
  return dpp<A>(a) OP b;\
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
template <unsigned A>\
constexpr auto operator OP (dpp<A> const& a,\
  detail::arithmetic auto const b) noexcept\
{\
  return a OP dpp<A>(b);\
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
constexpr auto abs(dpp<M> const& a) noexcept
{
  return a.mantissa() < 0 ? -a : a;
}

template <unsigned M>
constexpr auto trunc(dpp<M> const& a) noexcept
{
  if (!isnan(a) && (a.exponent() < 0))
  {
    auto m(a.mantissa());

    for (int_t e(a.exponent()); m && e; ++e, m /= 10);

    return dpp<M>(m, {}, direct{});
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
  dpp<M> const c{5, -1, direct{}};

  return !isnan(a) && (a.exponent() < 0) ?
    trunc(a.mantissa() < 0 ? a - c : a + c) :
    a;
}

//
template <unsigned M>
constexpr auto inv(dpp<M> const& a) noexcept
{
  using doubled_t = typename dpp<M>::doubled_t;

  enum : int_t
  {
    dp = detail::log10((long double)(detail::max_v<doubled_t>)) - 1
  };

  auto& m(a.v_.m);

  return !m || isnan(a) ?
    dpp<M>{nan{}} :
    dpp<M>{detail::pow<doubled_t, 10>(dp) / m, -dp - a.exponent()};
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

    auto const scandigit([&](auto const d) noexcept
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
          break;

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
constexpr std::optional<T> to_integral(dpp<M> const& p) noexcept
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

    if (int_t e(p.exponent()); e <= int_t{})
    {
      for (; m && e; ++e, m /= 10);
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
          return {}; // not representable
        }
      }
      while (--e);
    }

    return m;
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
inline auto& operator<<(std::ostream& os, dpp<M> const& p)
  noexcept(noexcept(os << to_string(p)))
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
    if (dpp::isnan(a))
    { // unique nan
      return dpp::detail::hash_combine(
        decltype(a.mantissa()){},
        dpp::int_t(dpp::dpp<M>::emin)
      );
    }
    else if (a.mantissa())
    {
      auto m(a.mantissa());
      dpp::int_t e(a.exponent());

      for (; !(m % 10); m /= 10, ++e); // slash zeros

      return dpp::detail::hash_combine(m, e);
    }
    else
    { // unique zero
      return dpp::detail::hash_combine(
        decltype(a.mantissa()){},
        dpp::int_t{}
      );
    }
  }
};

}

#endif // DPP_HPP
