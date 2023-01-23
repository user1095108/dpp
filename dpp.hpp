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

#include "intt/intt.hpp"

#if defined(_MSC_VER)
# define DPP_INT128T intt::intt<std::uint64_t, 2>
#else
# define DPP_INT128T __int128
#endif

namespace dpp
{

struct direct{};
struct nan{};

using int_t = std::int32_t; // int type wide enough to deal with exponents

namespace detail
{

template <typename U>
concept arithmetic =
  std::is_arithmetic_v<U> ||
  intt::intt_type<U> ||
  std::is_same_v<std::remove_cv_t<U>, DPP_INT128T>;

template <typename U>
concept integral =
  std::integral<U> ||
  intt::intt_type<U> ||
  std::is_same_v<std::remove_cv_t<U>, DPP_INT128T>;

template <typename U>
static constexpr auto is_signed_v(
  std::is_signed_v<U> ||
  intt::is_intt_v<U> ||
  std::is_same_v<std::remove_cv_t<U>, DPP_INT128T>
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

template <typename U>
consteval auto maxpow10e() noexcept
{
  int_t e{};

  for (U x(1); x <= detail::max_v<U> / 10; x *= U(10), ++e);

  return e;
}

}

template <typename T> requires(detail::is_signed_v<T>)
class dpp
{
public:
  using exp_type = std::int16_t;

  enum : exp_type
  {
    emin = detail::min_v<exp_type>,
    emax = detail::max_v<exp_type>
  };

  using mantissa_type = T;

  static constexpr auto mmin{detail::min_v<T>};
  static constexpr auto mmax{detail::max_v<T>};

  using doubled_t = std::conditional_t<
    std::is_same_v<T, std::int16_t>,
    std::int32_t,
    std::conditional_t<
      std::is_same_v<T, std::int32_t>,
      std::int64_t,
      std::conditional_t<
        std::is_same_v<T, std::int64_t>,
        DPP_INT128T,
        std::conditional_t<
          intt::is_intt_v<T>,
          typename intt::detail::double_<T>::type,
          void
        >
      >
    >
  >;

  enum : int_t { dp__ = detail::maxpow10e<doubled_t>() };

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
      if constexpr(detail::is_signed_v<U> &&
        (detail::bit_size_v<U> > detail::bit_size_v<T>))
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
      else if constexpr(std::is_unsigned_v<U> &&
        (detail::bit_size_v<U> >= detail::bit_size_v<T>))
      {
        if (m > mmax)
        {
          for (++e; m > 10 * U(mmax) - 5; m /= 10, ++e);

          m = (m + 5) / 10;
        }
      }

      //
      for (; (e <= emin) && m; m /= 10, ++e);

      //
      v_.e = (v_.m = m) ? e : 0;
    }
  }

  constexpr dpp(detail::integral auto const m) noexcept: dpp(m, {}) { }

  template <typename U>
  constexpr dpp(dpp<U> const& o) noexcept:
    dpp(o.mantissa(), o.exponent())
  {
  }

  constexpr dpp(std::floating_point auto f) noexcept
  {
    if (std::isfinite(f))
    {
      int_t e{};

      {
        decltype(f) const k(10);

        // eliminate the fractional part, slash f, if necessary
        for (; std::trunc(f) != f; f *= k, --e);
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

  template <std::floating_point U>
  explicit (sizeof(U) != sizeof(v_.m)) operator U() const noexcept
  {
    if (isnan(*this))
    {
      return NAN;
    }
    else if (auto m(v_.m); m)
    {
      int_t e(v_.e);

      for (; !(m % 10); m /= 10, ++e);

      return m * std::pow(U{10}, e);
    }
    else
    {
      return {};
    }
  }

  // this function is unsafe, take a look at to_integral() for safety
  template <detail::integral U>
  constexpr explicit operator U() const noexcept
  {
    if (auto e(v_.e); e < 0)
    {
      auto m(v_.m);

      for (; m && e; ++e, m /= 10);

      return m;
    }
    else
    {
      return v_.m * detail::pow<U, 10>(e);
    }
  }

  // assignment
  dpp& operator=(dpp const&) = default;
  dpp& operator=(dpp&&) = default;

  #define DPP_ASSIGNMENT__(OP)\
    template <typename U>\
    constexpr auto& operator OP ## =(U&& a) noexcept\
    {\
      return *this = *this OP std::forward<U>(a);\
    }

  DPP_ASSIGNMENT__(+)
  DPP_ASSIGNMENT__(-)
  DPP_ASSIGNMENT__(*)
  DPP_ASSIGNMENT__(/)

  // increment, decrement
  constexpr auto& operator++() noexcept
  {
    return *this += dpp{1, {}, direct{}};
  }

  constexpr auto& operator--() noexcept
  {
    return *this -= dpp{1, {}, direct{}};
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
    if (auto const om(o.v_.m); !om || isnan(*this) || isnan(o))
    {
      return nan{};
    }
    else
    {
      auto e(-dp__ + v_.e - o.v_.e);
      auto m(detail::pow<doubled_t, 10>(dp__) / o.v_.m);

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

using d64 = dpp<std::int64_t>;
using d32 = dpp<std::int32_t>;
using d16 = dpp<std::int16_t>;

// type promotions
#define DPP_TYPE_PROMOTION__(OP)\
template <typename A, typename B>\
constexpr auto operator OP (dpp<A> const& a, dpp<B> const& b) noexcept\
{\
  if constexpr(detail::bit_size_v<A> < detail::bit_size_v<B>)\
    return dpp<B>(a) OP b;\
  else\
    return a OP dpp<A>(b);\
}

DPP_TYPE_PROMOTION__(+)
DPP_TYPE_PROMOTION__(-)
DPP_TYPE_PROMOTION__(*)
DPP_TYPE_PROMOTION__(/)
DPP_TYPE_PROMOTION__(<=>)

template <typename A, typename B>
constexpr bool operator==(dpp<A> const& a, dpp<B> const& b) noexcept
{
  return a <=> b == 0;
}

// conversions
#define DPP_LEFT_CONVERSION__(OP)\
template <typename A>\
constexpr auto operator OP (detail::arithmetic auto const a,\
  dpp<A> const& b) noexcept\
{\
  return dpp<A>(a) OP b;\
}

DPP_LEFT_CONVERSION__(+)
DPP_LEFT_CONVERSION__(-)
DPP_LEFT_CONVERSION__(*)
DPP_LEFT_CONVERSION__(/)
DPP_LEFT_CONVERSION__(==)
DPP_LEFT_CONVERSION__(!=)
DPP_LEFT_CONVERSION__(<)
DPP_LEFT_CONVERSION__(<=)
DPP_LEFT_CONVERSION__(>)
DPP_LEFT_CONVERSION__(>=)
DPP_LEFT_CONVERSION__(<=>)

#define DPP_RIGHT_CONVERSION__(OP)\
template <typename A>\
constexpr auto operator OP (dpp<A> const& a,\
  detail::arithmetic auto const b) noexcept\
{\
  return a OP dpp<A>(b);\
}

DPP_RIGHT_CONVERSION__(+)
DPP_RIGHT_CONVERSION__(-)
DPP_RIGHT_CONVERSION__(*)
DPP_RIGHT_CONVERSION__(/)
DPP_RIGHT_CONVERSION__(==)
DPP_RIGHT_CONVERSION__(!=)
DPP_RIGHT_CONVERSION__(<)
DPP_RIGHT_CONVERSION__(<=)
DPP_RIGHT_CONVERSION__(>)
DPP_RIGHT_CONVERSION__(>=)
DPP_RIGHT_CONVERSION__(<=>)

// utilities
template <typename T>
constexpr auto isnan(dpp<T> const& a) noexcept
{
  return dpp<T>::emin == a.exponent();
}

//
template <typename T>
constexpr auto abs(dpp<T> const& a) noexcept
{
  return a.mantissa() < 0 ? -a : a;
}

//
template <typename T>
constexpr auto trunc(dpp<T> const& a) noexcept
{
  if (!isnan(a) && (a.exponent() < 0))
  {
    auto m(a.mantissa());

    for (int_t e(a.exponent()); m && e; ++e, m /= 10);

    return dpp<T>(m, {}, direct{});
  }
  else
  {
    return a;
  }
}

template <typename T>
constexpr auto ceil(dpp<T> const& a) noexcept
{
  auto const t(trunc(a));

  return t + (t < a);
}

template <typename T>
constexpr auto floor(dpp<T> const& a) noexcept
{
  auto const t(trunc(a));

  return t - (t > a);
}

template <typename T>
constexpr auto round(dpp<T> const& a) noexcept
{
  dpp<T> const c{5, -1, direct{}};

  return a.exponent() < 0 ?
    trunc(a.mantissa() < 0 ? a - c : a + c) :
    a;
}

//
template <typename T>
constexpr auto inv(dpp<T> const& a) noexcept
{
  using doubled_t = typename dpp<T>::doubled_t;

  auto const m(a.mantissa());

  return !m || isnan(a) ?
    dpp<T>{nan{}} :
    dpp<T>{
      detail::pow<doubled_t, 10>(dpp<T>::dp__) / m,
      -dpp<T>::dp__ - a.exponent()
    };
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

    bool neg{};

    switch (*i)
    {
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
      case '.':
        break;

      case '-':
        neg = true;
        [[fallthrough]];

      case '+':
        i = std::next(i);
        break;

      default:
        return nan{};
    }

    std::intmax_t r{};
    int_t e{};

    auto const scandigit([&](decltype(r) const d) noexcept
      {
        if (neg)
        {
          if (r >= min / 10)
          {
            if (auto const t(10 * r); t >= min + d)
            {
              r = t - d;

              return false;
            }
          }
        }
        else if (r <= max / 10)
        {
          if (auto const t(10 * r); t <= max - d)
          {
            r = t + d;

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

template <typename T>
constexpr auto to_decimal(auto const& s) noexcept ->
  decltype(std::cbegin(s), std::cend(s), T())
{
  return to_decimal<T>(std::cbegin(s), std::cend(s));
}

template <typename U = std::intmax_t, typename T>
constexpr std::optional<T> to_integral(dpp<T> const& p) noexcept
{
  enum : U
  {
    min = detail::min_v<U>,
    max = detail::max_v<U>
  };

  if (isnan(p))
  {
    return {};
  }
  else
  {
    auto m(p.mantissa());

    if (int_t e(p.exponent()); e <= 0)
    {
      for (; m && e; ++e, m /= 10);

      if ((m < min) || (m > max))
      {
        return {};
      }
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

template <typename T>
std::string to_string(dpp<T> const& a)
{
  if (isnan(a))
  {
    return {"nan", 3};
  }
  else
  {
    auto m(a.mantissa());
    int_t e(a.exponent());

    if (m)
    {
      for (; (e < 0) && !(m % 10); m /= 10, ++e);
    }
    else
    {
      e = {};
    }

    //
    std::string r;

    if constexpr(intt::is_intt_v<T>)
    {
      r = intt::to_string(m);
    }
    else
    {
      r = std::to_string(m);
    }

    if (e < 0)
    {
      auto const neg(m < 0);

      if (int_t const n(r.size() - neg + e); n > 0)
      {
        r.insert(n + neg, 1, '.');
      }
      else
      { // n <= 0
        r.insert(neg, std::string("0.", 2).append(-n, '0'));
      }
    }
    else // if (e > 0)
    {
      r.append(e, '0');
    }

    //
    return r;
  }
}

template <typename T>
inline auto& operator<<(std::ostream& os, dpp<T> const& p)
{
  return os << to_string(p);
}

//////////////////////////////////////////////////////////////////////////////
namespace literals
{

#define DPP_LITERAL__(ID)\
template <char ...c>\
constexpr auto operator "" _d ## ID() noexcept\
{\
  return to_decimal<d ## ID>((char const[sizeof...(c)]){c...});\
}

DPP_LITERAL__(16)
DPP_LITERAL__(32)
DPP_LITERAL__(64)

}

}

//////////////////////////////////////////////////////////////////////////////
namespace std
{

template <typename T>
struct hash<dpp::dpp<T>>
{
  auto operator()(dpp::dpp<T> const& a) const noexcept
  {
    if (dpp::isnan(a))
    { // unique nan
      return dpp::detail::hash_combine(
        decltype(a.mantissa()){},
        dpp::int_t(dpp::dpp<T>::emin)
      );
    }
    else if (auto m(a.mantissa()); m)
    {
      dpp::int_t e(a.exponent());

      for (; !(m % 10); m /= 10, ++e); // slash zeros

      return dpp::detail::hash_combine(m, e);
    }
    else
    { // unique zero
      return dpp::detail::hash_combine(decltype(m){}, dpp::int_t{});
    }
  }
};

}

#endif // DPP_HPP
