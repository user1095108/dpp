#ifndef DPP_HPP
# define DPP_HPP
# pragma once

#include <compare> // std::partial_ordering

#include "intt/intt.hpp"

#if defined(__SIZEOF_INT128__)
# define DPP_INT128T __int128
#else
# define DPP_INT128T intt::intt<std::uint64_t, 2>
#endif // __SIZEOF_INT128__

namespace dpp
{

struct direct{};
struct nan{};

namespace detail
{

template <typename U>
concept arithmetic =
  std::is_arithmetic_v<U> ||
  intt::intt_concept<U> ||
  std::is_same_v<std::remove_cv_t<U>, DPP_INT128T>;

template <typename U>
concept integral =
  std::integral<U> ||
  intt::intt_concept<U> ||
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
static constexpr U min_v(is_signed_v<U> ? ~U{} << (bit_size_v<U> - 1) : U{});

template <typename U> static constexpr U max_v(~min_v<U>);

template <typename U, typename E>
consteval auto maxpow10e() noexcept
{
  E e{};

  for (U x(1); x <= intt::coeff<detail::max_v<U> / U(10)>(); x *= U(10), ++e);

  return e;
}

consteval auto pow(auto&& x, auto&& e) noexcept
{
  std::remove_cvref_t<decltype(x)> r(1);

  pow(std::forward<decltype(x)>(x),
    std::forward<decltype(e)>(e),
    [&](auto&& x) noexcept { r *= x; }
  );

  return r;
}

constexpr void pow(auto x, auto e, auto const f) noexcept
  requires(std::is_same_v<void, decltype(f(x))>)
{
  for (;;)
  {
    if (e & decltype(e)(1)) f(x);

    if (e /= 2) [[likely]] x *= x; else [[unlikely]] return;
  }
}

constexpr void pow(auto x, auto e, auto const f) noexcept
  requires(std::is_same_v<bool, decltype(f(x))>)
{
  for (;;)
  {
    if ((e & decltype(e)(1)) && !f(x)) break;

    if (e /= 2) [[likely]] x *= x; else [[unlikely]] return;
  }
}

template <auto X, std::size_t E>
static constexpr auto pwrs{
  []<auto ...I>(std::index_sequence<I...>) noexcept
  {
    return std::array<decltype(X), E + 1>{pow(X, I)...};
  }(std::make_index_sequence<E + 1>())
};

template <typename U>
constexpr void align(auto& ma, auto& ea, decltype(ma) mb,
  std::remove_cvref_t<decltype(ea)> i) noexcept
{
  using I = std::remove_cvref_t<decltype(ea)>;
  using T = std::remove_cvref_t<decltype(ma)>;

  {
    auto const e0(std::min(i, intt::coeff<maxpow10e<U, I>()>()));

    i -= e0;
    ea -= e0;
    ma *= pwrs<T(10), maxpow10e<U, I>() + 1>[e0];
  }

  if (intt::is_neg(ma))
    for (; i && (ma >= intt::coeff<min_v<T> / 10>()); --i, --ea, ma *= T(10));
  else
    for (; i && (ma <= intt::coeff<max_v<T> / 10>()); --i, --ea, ma *= T(10));

  if (i)
  {
    mb /= pwrs<T(10), maxpow10e<U, I>() + 1>[
        std::min(i, intt::coeff<maxpow10e<U, I>() + 1>())
      ];
  }
}

template <typename U>
using double_t = std::conditional_t<
  std::is_same_v<U, std::int8_t>,
  std::int16_t,
  std::conditional_t<
    std::is_same_v<U, std::int16_t>,
    std::int32_t,
    std::conditional_t<
      std::is_same_v<U, std::int32_t>,
      std::int64_t,
      std::conditional_t<
        std::is_same_v<U, std::int64_t>,
        DPP_INT128T,
        std::conditional_t<
          intt::is_intt_v<U>,
          intt::detail::double_t<U>,
          void
        >
      >
    >
  >
>;

}

template <typename T, typename E>
  requires(detail::is_signed_v<T> && detail::is_signed_v<E>)
class dpp
{
public:
  using mantissa_type = T;

  static constexpr auto mmin{detail::min_v<T>};
  static constexpr auto mmax{detail::max_v<T>};

  using doubled_t = std::conditional_t<
    detail::bit_size_v<detail::double_t<T>> <= detail::bit_size_v<int>,
    int,
    detail::double_t<T>
  >;

  using exp_type = E;

  static constexpr auto emin{detail::min_v<E>};
  static constexpr auto emax{detail::max_v<E>};

  using int_t = std::conditional_t<
    detail::bit_size_v<detail::double_t<E>> <= detail::bit_size_v<int>,
    int,
    detail::double_t<E>
  >; // int type wide enough to deal with exponents

  static constexpr auto dp__{detail::maxpow10e<doubled_t, int_t>()};

public:
  struct { T m; E e; } v_;

public:
  dpp() = default;

  dpp(dpp const&) = default;
  dpp(dpp&&) = default;

  template <detail::integral U>
  constexpr dpp(U m, int_t e) noexcept
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
      if ((m > T{}) && (m > intt::coeff<U(mmax)>()))
      {
        for (++e; m > intt::coeff<U(10 * U(mmax) - 5)>(); m /= 10, ++e);

        m = (m + 5) / 10;
      }
    }

    //
    for (; (e <= intt::coeff<emin>()) && m; m /= 10, ++e);

    if (e <= intt::coeff<emax>()) [[likely]]
    {
      v_.e = (v_.m = m) ? E(e) : E{};
    }
    else [[unlikely]]
    {
      *this = nan{};
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
    if (std::isfinite(a)) [[likely]]
    {
      enum
      {
        bits = std::min(
            detail::sig_bit_size_v<decltype(a)>,
            detail::bit_size_v<mantissa_type> - 1
          )
      };

      int e2;

      a = std::ldexp(std::frexp(a, &e2), bits);
      e2 -= bits;

      //
      int const e10(std::ceil(e2 * .30102999566398119521373889472449302676f));

      a = std::ldexp(a, e2 - e10);

      e10 <= 0 ?
        detail::pow(decltype(a)(5), e10, [&](auto&& x) noexcept {a *= x;}) :
        detail::pow(decltype(a)(5), e10, [&](auto&& x) noexcept {a /= x;});

      *this = dpp(mantissa_type(a), e10);
    }
    else [[unlikely]]
    {
      *this = nan{};
    }
  }

  //
  constexpr dpp(mantissa_type const m, exp_type const e, direct) noexcept:
    v_{.m = m, .e = e}
  {
  }

  constexpr dpp(nan) noexcept: v_{.m = {}, .e = intt::coeff<emin>()} { }

  //
  constexpr explicit operator bool() const noexcept
  {
    return v_.m || isnan(*this);
  }

  template <std::floating_point U>
  constexpr explicit (sizeof(U) != sizeof(v_.m)) operator U() const noexcept
  {
    if (isnan(*this)) [[unlikely]]
    {
      return NAN;
    }
    else [[likely]]
    {
      int const e(
        std::ceil(int(v_.e) * 3.32192809488736234787031942948939017586483139f)
      );

      auto a(*this);

      e <= 0 ?
        detail::pow(dpp(2, {}, direct{}), e, [&](auto&& x)noexcept {a *= x;}):
        detail::pow(dpp(2, {}, direct{}), e, [&](auto&& x)noexcept {a /= x;});

      return std::ldexp(U(mantissa_type(a)), e);
    }
  }

  template <detail::integral U>
  constexpr explicit operator U() const noexcept
  { // this function is unsafe, take a look at to_integral() for safety
    if (intt::is_neg(v_.e))
    {
      using I = int_t;

      I e1(-I(v_.e)); // overflow prevention
      auto m(v_.m);

      do
      {
        I const e0(std::min(e1, intt::coeff<detail::maxpow10e<T, I>()>()));

        e1 -= e0;
        m /= detail::pwrs<T(10), detail::maxpow10e<T, I>()>[e0];
      }
      while (e1 && m);

      return m;
    }
    else
    {
      U m(v_.m);

      detail::pow(U(10), v_.e, [&](auto&& x) noexcept { m *= x; });

      return m;
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
    if (isnan(*this)) [[unlikely]] return dpp{nan{}}; else
      [[likely]] if (intt::coeff<mmin>() == v_.m) [[unlikely]]
        return dpp(intt::coeff<doubled_t(-doubled_t(mmin))>(), v_.e); else
        [[likely]] return dpp(-v_.m, v_.e, direct{});
  }

  constexpr dpp operator+(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o)) [[unlikely]]
    {
      return nan{};
    }
    else if (auto const m(v_.m); !m) [[unlikely]]
    {
      return o;
    }
    else if (auto const om(o.v_.m); !om) [[unlikely]]
    {
      return *this;
    }
    else [[likely]]
    {
      doubled_t ma(m), mb(om);

      if (int_t ea(v_.e), eb(o.v_.e); ea < eb)
      {
        detail::align<T>(mb, eb, ma, eb - ea);

        return {ma + mb, eb};
      }
      else
      {
        detail::align<T>(ma, ea, mb, ea - eb);

        return {ma + mb, ea};
      }
    }
  }

  constexpr dpp operator-(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o)) [[unlikely]]
    {
      return nan{};
    }
    else if (auto const m(v_.m), om(o.v_.m); !om) [[unlikely]]
    {
      return *this;
    }
    else if (auto const oe(o.v_.e); !m) [[unlikely]]
    { // prevent overflow
      return intt::coeff<mmin>() == om ?
        dpp(intt::coeff<doubled_t(-doubled_t(mmin))>(), oe) :
        dpp(-om, oe, direct{});
    }
    else [[likely]]
    {
      doubled_t ma(m), mb(om);

      if (int_t ea(v_.e), eb(oe); ea < eb)
      {
        detail::align<T>(mb, eb, ma, eb - ea);

        return {ma - mb, eb};
      }
      else
      {
        detail::align<T>(ma, ea, mb, ea - eb);

        return {ma - mb, ea};
      }
    }
  }

  constexpr dpp operator*(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o)) [[unlikely]] return nan{}; else
      [[likely]] return dpp(doubled_t(v_.m) * o.v_.m, int_t(v_.e) + o.v_.e);
  }

  constexpr dpp operator/(dpp const& o) const noexcept
  {
    if (!o.v_.m || isnan(*this) || isnan(o)) [[unlikely]]
    {
      return nan{};
    }
    else if (v_.m) [[likely]]
    {
      using U = doubled_t;

      constexpr auto e0(detail::maxpow10e<T, int_t>());

      auto e(intt::coeff<int_t(-e0)>() + v_.e - o.v_.e);
      auto m(v_.m * intt::coeff<detail::pow(U(10), e0)>());

      if (intt::is_neg(m))
        for (; m >= intt::coeff<detail::min_v<U> / 10>(); m *= U(10), --e);
      else
        for (; m <= intt::coeff<detail::max_v<U> / 10>(); m *= U(10), --e);

      return dpp(m / o.v_.m, e);
    }
    else
    {
      return {};
    }
  }

  //
  constexpr std::partial_ordering operator<=>(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o)) [[unlikely]]
    {
      return std::partial_ordering::unordered;
    }
    else if (auto const m(v_.m), om(o.v_.m); !m || !om) [[unlikely]]
    {
      return m <=> om;
    }
    else [[likely]]
    {
      doubled_t ma(m), mb(om);

      {
        int_t ea(v_.e), eb(o.v_.e); // important to prevent overflow

        ea < eb ?
          detail::align<T>(mb, eb, ma, eb - ea) :
          detail::align<T>(ma, ea, mb, ea - eb);
      }

      return ma <=> mb;
    }
  }

  //
  friend auto& operator<<(std::ostream& os, dpp const& p)
  {
    return os << to_string(p);
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
  constexpr auto& exponent() const noexcept { return v_.e; }
  constexpr auto& mantissa() const noexcept { return v_.m; }
};

using d256 = dpp<intt::intt<std::uint64_t, 4>, std::int32_t>;
using d128 = dpp<intt::intt<std::uint64_t, 2>, std::int32_t>;
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
  return intt::coeff<dpp<T, E>::emin>() == a.exponent();
}

//
template <typename T, typename E>
constexpr auto abs(dpp<T, E> const& a) noexcept
{
  return intt::is_neg(a.mantissa()) ? -a : a;
}

//
template <typename T, typename E>
constexpr auto trunc(dpp<T, E> const& a) noexcept
{
  return !intt::is_neg(a.exponent()) || isnan(a) ?
    a : dpp<T, E>(T(a), {}, direct{});
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
  dpp<T, E> const c(5, -1, direct{});

  return intt::is_neg(a.exponent()) ?
    trunc(intt::is_neg(a.mantissa()) ? a - c : a + c) :
    a;
}

//
template <typename T, typename E>
constexpr auto inv(dpp<T, E> const& a) noexcept
{
  using doubled_t = typename dpp<T, E>::doubled_t;
  using int_t = typename dpp<T, E>::int_t;

  auto const m(a.mantissa());

  if (!m || isnan(a)) [[unlikely]] return dpp<T, E>{nan{}}; else
    [[likely]] return dpp<T, E>{
      intt::coeff<detail::pow(doubled_t(10), dpp<T, E>::dp__)>() / m,
      intt::coeff<int_t(-dpp<T, E>::dp__)>() - a.exponent()
    };
}

// conversions
template <typename T>
constexpr T to_decimal(std::input_iterator auto i,
  decltype(i) const end) noexcept
{
  if (i == end) [[unlikely]]
  {
    return nan{};
  }
  else [[likely]]
  {
    bool neg{};

    switch (*i)
    {
      [[likely]] case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
      case '.':
        break;

      case '-':
        neg = true;
        [[fallthrough]];

      case '+':
        ++i;
        break;

      [[unlikely]] default:
        return nan{};
    }

    //
    typename T::mantissa_type r{};
    typename T::int_t e{};

    for (bool dcp{}; end != i; ++i)
    {
      switch (*i)
      {
        [[likely]] case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          if (intt::coeff<T::emin>() == e) [[unlikely]] break; else [[likely]]
          {
            if (r >= intt::coeff<T::mmin / 10>()) [[likely]]
            {
              if (decltype(r) const t(10 * r), d(*i - '0');
                t >= intt::coeff<T::mmin>() + d) [[likely]]
              {
                r = t - d;
                e -= dcp;

                continue;
              }
            }
          }

          break;

        case '.':
          if (dcp) [[unlikely]] return nan{}; else [[likely]]
            { dcp = true; continue; }

        case '\0':
          break;

        [[unlikely]] default:
          return nan{};
      }

      break;
    }

    //
    return {
      neg ? r : intt::coeff<T::mmin>() == r ? intt::coeff<T::mmax>() : -r,
      e
    };
  }
}

template <typename T>
constexpr auto to_decimal(auto const& s) noexcept ->
  decltype(std::cbegin(s), std::cend(s), T())
{
  return to_decimal<T>(std::cbegin(s), std::cend(s));
}

template <typename T, typename E>
std::string to_string(dpp<T, E> const& a)
{
  if (isnan(a)) [[unlikely]]
  {
    return {"nan", 3};
  }
  else [[likely]]
  {
    auto m(a.mantissa());
    typename dpp<T, E>::int_t e;

    if (m) [[likely]]
    {
      if (intt::is_neg(e = a.exponent())) for (; !(m % 10); m /= 10, ++e);
    }
    else [[unlikely]]
    {
      e = {};
    }

    //
    using intt::to_string;
    using std::to_string;

    auto r(to_string(m));

    if (intt::is_neg(e))
    {
      auto const neg(intt::is_neg(m));

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

//////////////////////////////////////////////////////////////////////////////
namespace literals
{

#define DPP_LITERAL__(ID)\
template <char ...c>\
constexpr auto operator "" _d ## ID() noexcept\
{\
  char const d[]{c...};\
  return to_decimal<d ## ID>(d);\
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
      noexcept(std::hash<T>()(std::declval<T>())) &&
      noexcept(std::hash<int_t>()(std::declval<int_t>()))
    )
  {
    T m;
    int_t e(a.exponent());

    if (dpp::isnan(a)) [[unlikely]]
    { // unique nan
      m = {};
    }
    else if ((m = a.mantissa())) [[likely]]
    { // unique everything
      for (; !(m % 10); m /= 10, ++e); // slash zeros
    }
    else [[unlikely]]
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
