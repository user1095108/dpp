#ifndef DPP_HPP
# define DPP_HPP
# pragma once

#include <compare> // std::partial_ordering

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
static constexpr std::size_t bit_size_v(CHAR_BIT * sizeof(U));

template <typename U>
static constexpr std::size_t sig_bit_size_v(
  std::is_same_v<U, float> ? 24 : std::is_same_v<U, double> ? 53 : 64
);

template <typename U>
static constexpr U min_v(is_signed_v<U> ? U(1) << (bit_size_v<U> - 1) : U{});

template <typename U>
static constexpr U max_v(is_signed_v<U> ? -(min_v<U> + U(1)) : ~U());

constexpr void shift_left(auto& m, auto& e,
  std::remove_cvref_t<decltype(e)> i) noexcept
{ // we need to be mindful of overflow, since we are shifting left
  if (m < 0)
  {
    for (;
      (m >= intt::coeff<min_v<std::remove_cvref_t<decltype(m)>> / 20>()) && i;
      --i, m *= 10, --e);
  }
  else
  {
    for (;
      (m <= intt::coeff<max_v<std::remove_cvref_t<decltype(m)>> / 20>()) && i;
      --i, m *= 10, --e);
  }
}

constexpr void shift_right(auto& m, auto i) noexcept
{
  for (; m && i; --i, m /= 10);
}

template <typename T, int B>
constexpr T pow(auto e) noexcept
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

template <typename U, typename E>
consteval auto maxpow10e() noexcept
{
  auto const k(detail::max_v<U> / U(10));

  E e{};

  for (U x(1); x <= k; x *= U(10), ++e);

  return e;
}

}

template <typename T, typename E>
  requires(detail::is_signed_v<T> && std::is_signed_v<E>)
class dpp
{
public:
  using exp_type = E;

  enum : exp_type
  {
    emin = detail::min_v<exp_type>,
    emax = detail::max_v<exp_type>
  };

  // int type wide enough to deal with exponents
  using int_t = std::conditional_t<
    std::is_same_v<E, std::int8_t>,
    std::int16_t,
    std::conditional_t<
      std::is_same_v<E, std::int16_t>,
      std::int32_t,
      std::conditional_t<
        std::is_same_v<E, std::int32_t>,
        std::int64_t,
        std::conditional_t<
          std::is_same_v<E, std::int64_t>,
          DPP_INT128T,
          void
        >
      >
    >
  >;

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

  enum : int_t { dp__ = detail::maxpow10e<doubled_t, int_t>() };

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
        if (m < intt::coeff<U(mmin)>())
        {
          for (++e; m < intt::coeff<U(10 * U(mmin) + 5)>(); m /= 10, ++e);

          m = (m - 5) / 10;
        }
        else if (m > intt::coeff<U(mmax)>())
        {
          for (++e; m > intt::coeff<U(10 * U(mmax) - 5)>(); m /= 10, ++e);

          m = (m + 5) / 10;
        }
      }
      else if constexpr(std::is_unsigned_v<U> &&
        (detail::bit_size_v<U> >= detail::bit_size_v<T>))
      {
        if (m > intt::coeff<U(mmax)>())
        {
          for (++e; m > intt::coeff<U(10 * U(mmax) - 5)>(); m /= 10, ++e);

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

  template <typename U, typename V>
  constexpr dpp(dpp<U, V> const& o) noexcept:
    dpp(o.mantissa(), o.exponent())
  {
  }

  constexpr dpp(std::floating_point auto a) noexcept
  {
    if (std::isfinite(a))
    {
      enum
      {
        bits = std::min(
            detail::sig_bit_size_v<decltype(a)>,
            detail::bit_size_v<std::intmax_t> - 1
          )
      };

      int e2;

      a = std::ldexp(std::frexp(a, &e2), bits);
      e2 -= bits;

      //
      int const e10(std::ceil(e2 * .30102999566398119521373889472449302676f));

      a = std::ldexp(a, e2 - e10);

      auto const b(detail::pow<decltype(a), 5>(std::abs(e10)));

      //
      *this = dpp(std::intmax_t(e10 <= 0 ? a * b : a / b), e10);
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
      int e10(v_.e);

      for (; !(m % 10); m /= 10, ++e10);

      auto const b(detail::pow<U, 5>(std::abs(e10)));

      return std::ldexp(e10 >= 0 ? m * b : m / b, e10);
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
    return *this += intt::coeff<dpp{1, {}, direct{}}>();
  }

  constexpr auto& operator--() noexcept
  {
    return *this -= intt::coeff<dpp{1, {}, direct{}}>();
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
      intt::coeff<mmin>() == v_.m ?
        dpp(intt::coeff<-doubled_t(mmin)>(), v_.e) :
        dpp(-v_.m, v_.e, direct{});
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
      return intt::coeff<mmin>() == om ?
        dpp(intt::coeff<-doubled_t(mmin)>(), oe) :
        dpp(-om, oe, direct{});
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
      dpp(doubled_t(v_.m) * o.v_.m, int_t(v_.e) + o.v_.e);
  }

  constexpr dpp operator/(dpp const& o) const noexcept
  {
    if (auto const om(o.v_.m); !om || isnan(*this) || isnan(o))
    {
      return nan{};
    }
    else
    {
      using U = doubled_t;

      int_t e(-dp__ + v_.e - o.v_.e);
      auto m(intt::coeff<detail::pow<U, 10>(int_t(dp__))>() / o.v_.m);

      if (m < intt::coeff<U(mmin)>())
      {
        for (++e; m < intt::coeff<U(10 * U(mmin) + 5)>(); m /= 10, ++e);

        m = (m - 5) / 10;
      }
      else if (m > intt::coeff<U(mmax)>())
      {
        for (++e; m > intt::coeff<U(10 * U(mmax) - 5)>(); m /= 10, ++e);

        m = (m + 5) / 10;
      }

      return dpp(m * v_.m, e);
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
  static constexpr dpp min() noexcept
  {
    return intt::coeff<dpp{mmin, emax, direct{}}>();
  }

  static constexpr dpp max() noexcept
  {
    return intt::coeff<dpp{mmax, emax, direct{}}>();
  }

  //
  constexpr auto exponent() const noexcept { return v_.e; }
  constexpr auto mantissa() const noexcept { return v_.m; }
};

using d256 = dpp<intt::intt<std::uint64_t, 4>, std::int16_t>;
using d128 = dpp<intt::intt<std::uint64_t, 2>, std::int16_t>;
using d96 = dpp<intt::intt<std::uint32_t, 3>, std::int16_t>;
using d64 = dpp<std::int64_t, std::int16_t>;
using d48 = dpp<intt::intt<std::uint16_t, 3>, std::int16_t>;
using d32 = dpp<std::int32_t, std::int16_t>;
using d24 = dpp<intt::intt<std::uint8_t, 3>, std::int8_t>;
using d16 = dpp<std::int16_t, std::int8_t>;

// type promotions
#define DPP_TYPE_PROMOTION__(OP)\
template <typename A, typename B, typename C, typename D>\
constexpr auto operator OP (dpp<A, B> const& a, dpp<C, D> const& b) noexcept\
{\
  if constexpr(detail::bit_size_v<A> < detail::bit_size_v<C>)\
    return dpp<C, D>(a) OP b;\
  else\
    return a OP dpp<A, B>(b);\
}

DPP_TYPE_PROMOTION__(+)
DPP_TYPE_PROMOTION__(-)
DPP_TYPE_PROMOTION__(*)
DPP_TYPE_PROMOTION__(/)
DPP_TYPE_PROMOTION__(<=>)

template <typename A, typename B, typename C, typename D>
constexpr bool operator==(dpp<A, B> const& a, dpp<C, D> const& b) noexcept
{
  return a <=> b == 0;
}

// conversions
#define DPP_LEFT_CONVERSION__(OP)\
template <typename A, typename B>\
constexpr auto operator OP (detail::arithmetic auto const a,\
  dpp<A, B> const& b) noexcept\
{\
  return dpp<A, B>(a) OP b;\
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
template <typename A, typename B>\
constexpr auto operator OP (dpp<A, B> const& a,\
  detail::arithmetic auto const b) noexcept\
{\
  return a OP dpp<A, B>(b);\
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
template <typename T, typename E>
constexpr auto isnan(dpp<T, E> const& a) noexcept
{
  return dpp<T, E>::emin == a.exponent();
}

//
template <typename T, typename E>
constexpr auto abs(dpp<T, E> const& a) noexcept
{
  return a.mantissa() < T{} ? -a : a;
}

//
template <typename T, typename E>
constexpr auto trunc(dpp<T, E> const& a) noexcept
{
  if (!isnan(a) && (a.exponent() < 0))
  {
    auto m(a.mantissa());

    for (typename dpp<T, E>::int_t e(a.exponent()); m && e; ++e, m /= 10);

    return dpp<T, E>(m, {}, direct{});
  }
  else
  {
    return a;
  }
}

template <typename T, typename E>
constexpr auto ceil(dpp<T, E> const& a) noexcept
{
  auto const t(trunc(a));

  return t + (t < a);
}

template <typename T, typename E>
constexpr auto floor(dpp<T, E> const& a) noexcept
{
  auto const t(trunc(a));

  return t - (t > a);
}

template <typename T, typename E>
constexpr auto round(dpp<T, E> const& a) noexcept
{
  auto const c(intt::coeff<dpp<T, E>{5, -1, direct{}}>());

  return a.exponent() < 0 ?
    trunc(a.mantissa() < T{} ? a - c : a + c) :
    a;
}

//
template <typename T, typename E>
constexpr auto inv(dpp<T, E> const& a) noexcept
{
  using doubled_t = typename dpp<T, E>::doubled_t;
  using int_t = typename dpp<T, E>::int_t;

  auto const m(a.mantissa());

  return !m || isnan(a) ?
    dpp<T, E>{nan{}} :
    dpp<T, E>{
      intt::coeff<detail::pow<doubled_t, 10>(int_t(dpp<T, E>::dp__))>() / m,
      -dpp<T, E>::dp__ - a.exponent()
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
    typename T::int_t e{};

    auto const scandigit([&](decltype(r) const d) noexcept
      {
        if (r >= intt::coeff<min / 10>())
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

    return T(neg ? r : -(min == r ? r / 10 : r), e + (!neg && (min == r)));
  }
}

template <typename T>
constexpr auto to_decimal(auto const& s) noexcept ->
  decltype(std::cbegin(s), std::cend(s), T())
{
  return to_decimal<T>(std::cbegin(s), std::cend(s));
}

template <typename U = std::intmax_t, typename T, typename E>
constexpr auto to_integral(dpp<T, E> const& p) noexcept
{
  if (isnan(p))
  {
    return std::pair(p, true);
  }
  else
  {
    auto m(p.mantissa());

    if (typename dpp<T, E>::int_t e(p.exponent()); e <= 0)
    {
      for (; m && e; ++e, m /= 10);

      if ((m < intt::coeff<detail::min_v<U>>()) ||
        (m > intt::coeff<detail::max_v<U>>()))
      {
        return std::pair(m, true);
      }
    }
    else
    {
      do
      {
        if ((m >= intt::coeff<detail::min_v<U> / 10>()) &&
          (m <= intt::coeff<detail::max_v<U> / 10>()))
        {
          m *= 10;
        }
        else
        {
          return std::pair(m, true);
        }
      }
      while (--e);
    }

    return std::pair(m, false);
  }
}

template <typename T, typename E>
std::string to_string(dpp<T, E> const& a)
{
  if (isnan(a))
  {
    return {"nan", 3};
  }
  else
  {
    auto m(a.mantissa());
    typename dpp<T, E>::int_t e(a.exponent());

    if (m)
    {
      for (; (e < 0) && !(m % 10); m /= 10, ++e);
    }
    else
    {
      e = {};
    }

    //
    using intt::to_string;
    using std::to_string;

    auto r(to_string(m));

    if (e < 0)
    {
      auto const neg(m < T{});

      if (decltype(e) const n(r.size() - neg + e); n > 0)
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

template <typename T, typename E>
inline auto& operator<<(std::ostream& os, dpp<T, E> const& p)
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
DPP_LITERAL__(24)
DPP_LITERAL__(32)
DPP_LITERAL__(48)
DPP_LITERAL__(64)
DPP_LITERAL__(96)
DPP_LITERAL__(128)
DPP_LITERAL__(256)

}

}

//////////////////////////////////////////////////////////////////////////////
namespace std
{

template <typename T, typename E>
struct hash<dpp::dpp<T, E>>
{
  using int_t = typename dpp::dpp<T, E>::int_t;

  constexpr auto operator()(dpp::dpp<T, E> const& a)
    noexcept(
      noexcept(std::declval<std::hash<T>>()(std::declval<T>())) &&
      noexcept(
        std::declval<std::hash<int_t>>()(std::declval<int_t>())
      )
    )
  {
    T m;
    int_t e(a.exponent());

    if (dpp::isnan(a))
    { // unique nan
      m = {};
    }
    else if ((m = a.mantissa()))
    { // unique everything
      for (; !(m % 10); m /= 10, ++e); // slash zeros
    }
    else
    { // unique zero
      e = {};
    }

    //
    auto const s(std::hash<decltype(m)>()(m));

    return s ^ (std::hash<decltype(e)>()(e) + intt::magic::ISR +
      (s << 6) + (s >> 2));
  }
};

}

#endif // DPP_HPP
