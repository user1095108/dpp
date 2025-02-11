#ifndef DPP_HPP
# define DPP_HPP
# pragma once

#include <float.h>
#include "intt/intt.hpp"

#if defined(__SIZEOF_INT128__)
# define DPP_INT128T __int128
#else
# define DPP_INT128T intt::intt<std::uint64_t, 2>
#endif // __SIZEOF_INT128__

namespace dpp
{

struct direct_t { explicit direct_t() = default; };
inline constexpr direct_t direct{};

struct nan_t { explicit nan_t() = default; };
inline constexpr nan_t nan{};

namespace detail
{

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

template <typename U>
concept arithmetic =
  std::is_arithmetic_v<U> ||
  intt::is_intt_v<U> ||
  std::is_same_v<std::remove_cv_t<U>, DPP_INT128T>;

template <typename U>
concept integral =
  std::is_integral_v<U> ||
  intt::is_intt_v<U> ||
  std::is_same_v<std::remove_cv_t<U>, DPP_INT128T>;

template <typename U>
inline constexpr auto is_signed_v(
  std::is_signed_v<U> ||
  intt::is_intt_v<U> ||
  std::is_same_v<std::remove_cv_t<U>, DPP_INT128T>);

template <std::floating_point U>
inline constexpr std::size_t sig_bit_size_v(
  std::is_same_v<std::remove_cv_t<U>, float> ? FLT_MANT_DIG :
  std::is_same_v<std::remove_cv_t<U>, double> ? DBL_MANT_DIG : LDBL_MANT_DIG);

template <typename U>
inline constexpr U min_v(
  is_signed_v<U> ? ~U{} << (ar::bit_size_v<U> - 1) : U{});

template <typename U> inline constexpr U max_v(~min_v<U>);

template <auto B, typename E = std::size_t> requires(integral<decltype(B)>)
consteval E log(decltype(B) x) noexcept
{
  x /= B; E e{}; for (decltype(B) y(1); y <= x;) ++e, y *= B; return e;
}

template <typename U, typename E = std::size_t>
consteval auto maxpow10e() noexcept
{
  return log<U(10), E>(max_v<U>);
}

template <typename U, typename E = std::size_t>
consteval auto maxpow2e() noexcept
{
  return log<U(2), E>(maxpow10e<U, E>());
}

template <typename U, typename T, typename E = std::size_t>
consteval auto maxpow2e2() noexcept
{
  return log<U(2), E>(log<U(10), E>(max_v<U> >> (ar::bit_size_v<T> - 1)));
}

constexpr void pow(auto x, auto e, auto const f) noexcept
{
  for (;;)
  {
    if (e & decltype(e)(1)) f(x);

    if (e /= decltype(e)(2)) [[likely]] x *= x; else [[unlikely]] return;
  }
}

consteval auto pow(auto const x, auto const e) noexcept
{
  auto r(decltype(x)(1));

  pow(x, e, [&](auto&& x) noexcept { r *= x; });

  return r;
}

template <typename T>
constexpr void align(auto& ma, auto& ea, decltype(ma) mb,
  std::remove_reference_t<decltype(ea)> i) noexcept
{
  using U = std::remove_reference_t<decltype(ma)>;
  using F = std::remove_reference_t<decltype(ea)>;

  if (intt::is_neg(ma))
    //for (; i && (ma >= ar::coeff<min_v<U> / 10>()); --i, --ea, ma *= T(10));
    [&]<auto ...I>(std::index_sequence<I...>) noexcept
    {
      (
        [&]() noexcept -> bool
        {
          constexpr auto e(ar::coeff<pow(F(2), maxpow2e<U>() - I)>());
          constexpr auto f(ar::coeff<pow(U(10), e)>());
          constexpr auto cmp(ar::coeff<min_v<U> / f>());

          if ((e <= i) && (ma >= cmp)) i -= e, ea -= e, ma *= f;

          return i && (ma >= ar::coeff<U(min_v<U> / 10)>());
        }() && ...
      );
    }(std::make_index_sequence<maxpow2e<U>() + 1>());
  else
    //for (; i && (ma <= ar::coeff<max_v<U> / 10>()); --i, --ea, ma *= T(10));
    [&]<auto ...I>(std::index_sequence<I...>) noexcept
    {
      (
        [&]() noexcept -> bool
        {
          constexpr auto e(ar::coeff<pow(F(2), maxpow2e<U>() - I)>());
          constexpr auto f(ar::coeff<pow(U(10), e)>());
          constexpr auto cmp(ar::coeff<max_v<U> / f>());

          if ((e <= i) && (ma <= cmp)) i -= e, ea -= e, ma *= f;

          return i && (ma <= ar::coeff<U(max_v<U> / 10)>());
        }() && ...
      );
    }(std::make_index_sequence<maxpow2e<U>() + 1>());

  if (i)
    [&]<auto ...I>(std::index_sequence<I...>) noexcept
    {
      (
        [&]() noexcept -> bool
        {
          constexpr auto e(ar::coeff<pow(F(2), maxpow2e<T>() - I)>());

          if (e <= i) i -= e, mb /= ar::coeff<pow(U(10), e)>();

          return i && mb;
        }() && ...
      );
    }(std::make_index_sequence<maxpow2e<T>() + 1>());
}

}

template <typename T, typename E>
  requires(detail::is_signed_v<T> && detail::is_signed_v<E>)
struct dpp
{
  using sig_t = T;

  static constexpr auto mmax{detail::max_v<T>};
  static constexpr auto mmin{T(-mmax)};

  using sig2_t = std::conditional_t<
      ar::bit_size_v<detail::double_t<T>> <= ar::bit_size_v<std::int64_t>,
      std::int64_t,
      detail::double_t<T>
    >;

  using exp_t = E;

  static constexpr auto emin{detail::min_v<E>};
  static constexpr auto emax{detail::max_v<E>};

  using exp2_t = std::conditional_t<
      ar::bit_size_v<detail::double_t<E>> <= ar::bit_size_v<int>,
      int,
      detail::double_t<E>
    >; // int type wide enough to deal with exponents

  T m_; E e_;

  dpp() = default;

  dpp(dpp const&) = default;
  dpp(dpp&&) = default;

  constexpr dpp(sig2_t m, exp2_t e = {}) noexcept
  {
    using U = sig2_t;
    using F = exp2_t;

    using namespace detail;

    if (m < ar::coeff<U(mmin)>())
    { //for (++e; m < ar::coeff<U(10 * U(mmin) + 5)>(); ++e, m /= 10);
      [&]<auto ...I>(std::index_sequence<I...>) noexcept
      {
        (
          [&]() noexcept
          {
            constexpr auto J(ar::coeff<maxpow2e2<U, T>() - I>());
            constexpr auto e0(ar::coeff<pow(F(2), J)>());
            constexpr auto f(ar::coeff<pow(U(10), e0)>());
            constexpr auto cmp(ar::coeff<U(mmin) * f + (J ? 0 : 5)>());

            if (m < cmp) e += e0, m /= f;

            return m < ar::coeff<U(10 * U(mmin) + 5)>();
          }() && ...
        );
      }(std::make_index_sequence<maxpow2e2<U, T>() + 1>());

      ++e; m = (m - U(5)) / U(10);
    }
    else if (m > ar::coeff<U(mmax)>())
    { //for (++e; m > ar::coeff<U(10 * U(mmax) - 5)>(); ++e, m /= 10);
      [&]<auto ...I>(std::index_sequence<I...>) noexcept
      {
        (
          [&]() noexcept -> bool
          {
            constexpr auto J(ar::coeff<maxpow2e2<U, T>() - I>());
            constexpr auto e0(ar::coeff<pow(F(2), J)>());
            constexpr auto f(ar::coeff<pow(U(10), e0)>());
            constexpr auto cmp(ar::coeff<U(mmax) * f - (J ? 0 : 5)>());

            if (m > cmp) e += e0, m /= f;

            return m > ar::coeff<U(10 * U(mmax) - 5)>();
          }() && ...
        );
      }(std::make_index_sequence<maxpow2e2<U, T>() + 1>());

      ++e; m = (m + U(5)) / U(10);
    }

    //
    if (e <= ar::coeff<F(emax)>()) [[likely]]
    {
      //while ((e <= ar::coeff<F(emin)>()) && m) ++e, m /= 10;
      if (e <= ar::coeff<F(emin)>())
        [&]<auto ...I>(std::index_sequence<I...>) noexcept
        {
          (
            [&]() noexcept
            {
              constexpr auto e0(ar::coeff<pow(F(2), maxpow2e<T>() - I)>());

              if (auto const e1(e0 + e); e1 <= ar::coeff<F(emin + 1)>())
                e = e1, m /= ar::coeff<pow(U(10), e0)>();

              return (e <= ar::coeff<F(emin)>()) && m;
            }() && ...
          );
        }(std::make_index_sequence<maxpow2e<T>() + 1>());

      e_ = (m_ = m) ? E(e) : E{};
    }
    else [[unlikely]]
    {
      *this = nan;
    }
  }

  template <detail::integral U>
  constexpr dpp(U m, exp2_t e = {}) noexcept
  { // we need extra bits, hence exp2_t
    if constexpr(detail::is_signed_v<U> &&
      (ar::bit_size_v<U> > ar::bit_size_v<T>))
    {
      if (m < ar::coeff<U(mmin)>())
      {
        for (++e; m < ar::coeff<U(10 * U(mmin) + 5)>(); ++e, m /= 10);

        m = (m - U(5)) / U(10);
      }
      else if (m > ar::coeff<U(mmax)>())
      {
        for (++e; m > ar::coeff<U(10 * U(mmax) - 5)>(); ++e, m /= 10);

        m = (m + U(5)) / U(10);
      }
    }
    else if constexpr(std::is_unsigned_v<U> &&
      (ar::bit_size_v<U> >= ar::bit_size_v<T>))
    {
      if (m > ar::coeff<U(mmax)>())
      {
        for (++e; m > ar::coeff<U(10 * U(mmax) - 5)>(); ++e, m /= 10);

        m = (m + U(5)) / U(10);
      }
    }

    //
    if (e <= ar::coeff<exp2_t(emax)>()) [[likely]]
    {
      while ((e <= ar::coeff<exp2_t(emin)>()) && m) ++e, m /= 10;

      e_ = (m_ = m) ? E(e) : E{};
    }
    else [[unlikely]]
    {
      *this = nan;
    }
  }

  template <typename U, typename V>
  constexpr dpp(dpp<U, V> const& o) noexcept: dpp(o.sig(), o.exp()) { }

  constexpr dpp(std::floating_point auto a) noexcept
  {
    if (std::isfinite(a)) [[likely]]
    {
      enum
      {
        bits = std::min(
            detail::sig_bit_size_v<decltype(a)>,
            ar::bit_size_v<sig_t> - 1
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

      *this = dpp(sig_t(a), e10);
    }
    else [[unlikely]]
      *this = nan;
  }

  //
  constexpr dpp(direct_t, sig_t const& m) noexcept: m_(m), e_{} { }

  constexpr dpp(direct_t, sig_t const& m, exp_t const& e) noexcept:
    m_(m), e_(e)
  {
  }

  constexpr dpp(nan_t) noexcept: e_(ar::coeff<emin>())
  {
    if (std::is_constant_evaluated()) m_ = {};
  }

  //
  constexpr explicit operator bool() const noexcept
  {
    return m_ || isnan(*this);
  }

  template <std::floating_point U>
  constexpr explicit ((sizeof(U) != sizeof(m_)) ||
    !(std::is_same_v<U, float> || std::is_same_v<U, double> ||
    std::is_same_v<U, long double>))
  operator U() const noexcept
  {
    if (isnan(*this)) [[unlikely]]
      return NAN;
    else [[likely]]
    {
      int const e2(
        std::ceil(int(e_) * 3.32192809488736234787031942948939017586483139f)
      );

      auto a(*this);

      e2 <= 0 ?
        detail::pow(dpp(direct, T(2)), e2, [&](auto&& x) noexcept {a *= x;}):
        detail::pow(dpp(direct, T(2)), e2, [&](auto&& x) noexcept {a /= x;});

      return std::ldexp(U(sig_t(a)), e2);
    }
  }

  template <detail::integral U>
  constexpr explicit operator U() const noexcept
  { // this function is unsafe, take a look at to_integral() for safety
    using F = exp2_t;

    if (intt::is_neg(e_))
    {
      using namespace detail;

      auto m(m_);

      [&]<auto ...I>(std::index_sequence<I...>) noexcept
      {
        F e(e_);

        (
          [&]() noexcept -> bool
          {
            constexpr auto e0(ar::coeff<F(-pow(F(2), maxpow2e<T>() - I))>());

            if (e0 >= e) e -= e0, m /= ar::coeff<pow(T(10), e0)>();

            return e && m;
          }() && ...
        );
      }(std::make_index_sequence<maxpow2e<T>() + 1>());

      return m;
    }
    else
    {
      U m(m_);

      detail::pow(U(10), e_, [&](auto&& x) noexcept { m *= x; });

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
    return *this += ar::coeff<dpp{direct, T(1)}>();
  }

  constexpr auto& operator--() noexcept
  {
    return *this -= ar::coeff<dpp{direct, T(1)}>();
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
  constexpr dpp operator+() const noexcept { return *this; }

  constexpr dpp operator-() const noexcept { return dpp(direct, -m_, e_); }

  constexpr dpp operator+(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o)) [[unlikely]]
      return nan;
    else if (!m_) [[unlikely]]
      return o;
    else if (!o.m_) [[unlikely]]
      return *this;
    else [[likely]]
    {
      sig2_t ma(m_), mb(o.m_);
      exp2_t ea(e_), eb(o.e_);

      return ea < eb ?
        (detail::align<T>(mb, eb, ma, eb - ea), dpp(ma + mb, eb)) :
        (detail::align<T>(ma, ea, mb, ea - eb), dpp(ma + mb, ea));
    }
  }

  constexpr dpp operator-(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o)) [[unlikely]]
      return nan;
    else if (!m_) [[unlikely]]
      return dpp(direct, -o.m_, o.e_);
    else if (!o.m_) [[unlikely]]
      return *this;
    else [[likely]]
    {
      sig2_t ma(m_), mb(o.m_);
      exp2_t ea(e_), eb(o.e_);

      return ea < eb ?
        (detail::align<T>(mb, eb, ma, eb - ea), dpp(ma - mb, eb)) :
        (detail::align<T>(ma, ea, mb, ea - eb), dpp(ma - mb, ea));
    }
  }

  constexpr dpp operator*(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o)) [[unlikely]] return nan; else [[likely]]
      return dpp(sig2_t(m_) * sig2_t(o.m_), exp2_t(e_) + exp2_t(o.e_));
  }

  constexpr dpp operator/(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o) || !o.m_) [[unlikely]]
      return nan;
    else if (m_) [[likely]]
    {
      using U = sig2_t;
      using F = exp2_t;

      using namespace detail;

      constexpr auto e0(ar::coeff<maxpow10e<T, F>()>());

      auto e(F(e_) - F(o.e_) - e0);
      auto m(ar::coeff<pow(U(10), e0)>() * U(m_));

      if (intt::is_neg(m))
        //for (; m >= ar::coeff<detail::min_v<U> / 10>(); m *= U(10), --e);
        (
          [&]<auto ...I>(std::index_sequence<I...>) noexcept
          {
            (
              [&]() noexcept -> bool
              {
                constexpr auto e0(ar::coeff<pow(F(2), maxpow2e<T>() - I)>());
                constexpr auto f(ar::coeff<pow(U(10), e0)>());

                if (m >= ar::coeff<min_v<U> / f>()) e -= e0, m *= f;

                return m >= ar::coeff<U(min_v<U> / 10)>();
              }() && ...
            );
          }(std::make_index_sequence<maxpow2e<T>() + 1>())
        );
      else
        //for (; m <= ar::coeff<detail::max_v<U> / 10>(); m *= U(10), --e);
        [&]<auto ...I>(std::index_sequence<I...>) noexcept
        {
          (
            [&]() noexcept -> bool
            {
              constexpr auto e0(ar::coeff<pow(F(2), maxpow2e<T>() - I)>());
              constexpr auto f(ar::coeff<pow(U(10), e0)>());

              if (m <= ar::coeff<max_v<U> / f>()) e -= e0, m *= f;

              return m <= ar::coeff<U(max_v<U> / 10)>();
            }() && ...
          );
        }(std::make_index_sequence<maxpow2e<T>() + 1>());

      return dpp(m / sig2_t(o.m_), e);
    }
    else [[unlikely]]
      return {};
  }

  //
  constexpr std::partial_ordering operator<=>(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o)) [[unlikely]]
      return std::partial_ordering::unordered;
    else if (!m_ || !o.m_) [[unlikely]]
      return m_ <=> o.m_;
    else [[likely]]
    {
      sig2_t ma(m_), mb(o.m_);

      {
        exp2_t ea(e_), eb(o.e_); // important to prevent overflow

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
#if !defined(__clang__)
  static constexpr dpp eps{sig2_t(1), -detail::maxpow10e<T, exp2_t>()};
  static constexpr dpp max{direct, mmax, emax};
  static constexpr dpp min{direct, mmin, emax};
#endif

  //
  constexpr auto& exp() const noexcept { return e_; }
  constexpr auto& sig() const noexcept { return m_; }
};

using d1024 = dpp<intt::intt<std::uint64_t, 16>, std::int32_t>;
using d512 = dpp<intt::intt<std::uint64_t, 8>, std::int32_t>;
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
  if constexpr(ar::bit_size_v<A> < ar::bit_size_v<C>)\
    return dpp<C, D>(a) OP b;\
  else\
    return a OP dpp<A, B>(b);\
}

DPP_TYPE_PROMOTION__(+)
DPP_TYPE_PROMOTION__(-)
DPP_TYPE_PROMOTION__(*)
DPP_TYPE_PROMOTION__(/)
DPP_TYPE_PROMOTION__(<=>)

// comparisons
template <typename A, typename B>
constexpr bool operator==(dpp<A, B> const& a, nan_t) noexcept
{
  return isnan(a);
}

template <typename A, typename B>
constexpr bool operator==(nan_t, dpp<A, B> const& a) noexcept
{
  return isnan(a);
}

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
  return ar::coeff<dpp<T, E>::emin>() == a.exp();
}

//
template <typename T, typename E>
constexpr auto abs(dpp<T, E> const& a) noexcept
{
  return intt::is_neg(a.sig()) ? -a : a;
}

//
template <typename T, typename E>
constexpr auto trunc(dpp<T, E> const& a) noexcept
{
  return !intt::is_neg(a.exp()) || isnan(a) ? a : dpp<T, E>(direct, T(a));
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
  dpp<T, E> const c(direct, T(5), E(-1));

  return intt::is_neg(a.exp()) ?
    trunc(intt::is_neg(a.sig()) ? a - c : a + c) :
    a;
}

//
template <typename T, typename E>
constexpr dpp<T, E> inv(dpp<T, E> const& a) noexcept
{ // multiplicative inverse or reciprocal
  using U = typename dpp<T, E>::sig2_t;
  using F = typename dpp<T, E>::exp2_t;

  constexpr auto e0{ar::coeff<F(-detail::maxpow10e<U, F>())>()};

  if (isnan(a) || !a.m_) [[unlikely]] return nan; else
    [[likely]] return dpp<T, E>{
        ar::coeff<detail::pow(U(10), e0)>() / U(a.m_), e0 - F(a.e_)
      };
}

// conversions
template <typename T>
constexpr T to_decimal(std::input_iterator auto i,
  decltype(i) const end) noexcept
{
  if (i == end) [[unlikely]]
  {
    return nan;
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
        return nan;
    }

    //
    typename T::sig_t r{};
    typename T::exp_t e{};

    for (bool dcp{}; end != i; ++i)
    {
      switch (*i)
      {
        [[likely]] case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          if ((ar::coeff<typename T::exp_t(T::emin + 1)>() != e) &&
            (ar::coeff<typename T::exp_t(T::emax)>() != e))
          {
            if (r < ar::coeff<T::mmin / 10>()) [[unlikely]] ++e -= dcp;
            else if (decltype(r) const t(10 * r), d(*i - '0');
              t >= ar::coeff<T::mmin>() + d) [[likely]] e -= dcp, r = t - d;
            else [[unlikely]] ++e -= dcp, r = ar::coeff<T::mmin>();

            continue;
          }
          else
            return nan;

        case '.':
          if (dcp) [[unlikely]] return nan; else [[likely]]
            { dcp = true; continue; }

        case '\0':
          break;

        [[unlikely]] default:
          return nan;
      }

      break;
    }

    //
    return {direct, neg ? r : typename T::sig_t(-r), e};
  }
}

template <typename T>
constexpr auto to_decimal(auto const& s) noexcept ->
  decltype(std::begin(s), std::end(s), T())
{
  return to_decimal<T>(std::begin(s), std::end(s));
}

template <typename T, typename E>
std::string to_string(dpp<T, E> const& a)
{
  using F = typename dpp<T, E>::exp2_t;

  if (isnan(a)) [[unlikely]]
    return {"nan", 3};
  else [[likely]]
  {
    auto m(a.sig());
    F e;

    if (m) [[likely]]
    {
      if (intt::is_neg(e = a.exp()))
      { // for (; !(m % 10); ++e, m /= 10);
        using namespace detail;

        [&]<auto ...I>(std::index_sequence<I...>) noexcept
        { // slash zeros
          (
            [&]() noexcept -> bool
            {
              constexpr auto e0(ar::coeff<pow(F(2), maxpow2e<T>() - I)>());
              constexpr auto f(ar::coeff<pow(T(10), e0)>());

              if (!(m % f)) e += e0, m /= f;

              return !(m % T(10));
            }() && ...
          );
        }(std::make_index_sequence<maxpow2e<T>() + 1>());
      }
    }
    else [[unlikely]]
      e = {};

    //
    using intt::to_string;
    using std::to_string;

    auto r(to_string(m));

    if (intt::is_neg(e))
    {
      auto const neg(intt::is_neg(m));
      F const n(r.size() - neg + e);

      n > 0 ?
        r.insert(n + neg, 1, '.') :
        r.insert(neg, std::string("0.", 2).append(-n, '0'));
    }
    else // if (e > 0)
      r.append(e, '0');

    //
    return r;
  }
}

//////////////////////////////////////////////////////////////////////////////
namespace literals
{

#define DPP_LITERAL__(ID)\
template <char ...c>\
constexpr auto operator ""_d##ID() noexcept\
{\
  return to_decimal<d##ID>(std::initializer_list<char>{c...});\
}

DPP_LITERAL__(16) DPP_LITERAL__(24) DPP_LITERAL__(32) DPP_LITERAL__(48)
DPP_LITERAL__(64) DPP_LITERAL__(96) DPP_LITERAL__(128) DPP_LITERAL__(256)
DPP_LITERAL__(512) DPP_LITERAL__(1024)

}

}

//////////////////////////////////////////////////////////////////////////////
namespace std
{

template <typename T, typename E>
struct hash<dpp::dpp<T, E>>
{
  using U = typename dpp::dpp<T, E>::sig2_t;
  using F = typename dpp::dpp<T, E>::exp2_t;

  constexpr auto operator()(dpp::dpp<T, E> const& a)
    noexcept(noexcept(std::hash<T>()(std::declval<T>()),
      std::hash<F>()(std::declval<F>())))
  {
    T m;
    F e(a.exp());

    if (dpp::isnan(a)) [[unlikely]] // unique nan
      m = {};
    else if ((m = a.sig())) [[likely]]
    { // unique everything
      // for (; !(m % 10); ++e, m /= 10); // slash zeros
      using namespace dpp::detail;
      using dpp::detail::pow;

      [&]<auto ...I>(std::index_sequence<I...>) noexcept
      { // slash zeros
        (
          [&]() noexcept -> bool
          {
            constexpr auto e0(ar::coeff<pow(F(2), maxpow2e<T>() - I)>());
            constexpr auto f(ar::coeff<pow(T(10), e0)>());

            if (!(m % f)) e += e0, m /= f;

            return !(m % T(10));
          }() && ...
        );
      }(std::make_index_sequence<maxpow2e<T>() + 1>());
    }
    else [[unlikely]] // unique zero
      e = {};

    //
    auto const s(std::hash<decltype(m)>()(m));

    return s ^ (std::hash<F>()(e) + intt::consts::ISR + (s << 6) + (s >> 2));
  }
};

}

#endif // DPP_HPP
