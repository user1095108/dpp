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
  std::is_same_v<U, float> ? FLT_MANT_DIG :
  std::is_same_v<U, double> ? DBL_MANT_DIG : LDBL_MANT_DIG);

template <typename U>
inline constexpr U min_v(
  is_signed_v<U> ? ~U{} << (ar::bit_size_v<U> - 1) : U{});

template <typename U> inline constexpr U max_v(~min_v<U>);

template <auto B, typename E = std::size_t>
consteval E log(decltype(B) x) noexcept
{
  x /= B; E e{}; for (decltype(B) y(1); y <= x;) ++e, y *= B; return e;
}

template <typename U, typename E = std::size_t>
consteval auto maxpow10e() noexcept
{
  return log<U(10), E>(max_v<U>);
}

template <typename U>
inline constexpr auto logend(ar::coeff<log<U(2)>(maxpow10e<U>())>());

consteval auto pow(auto const x, auto const e) noexcept
{
  std::remove_const_t<decltype(x)> r(1);

  pow(x, e, [&](auto&& x) noexcept { r *= x; });

  return r;
}

constexpr void pow(auto x, auto e, auto const f) noexcept
  requires(std::same_as<void, decltype(f(x))>)
{
  for (;;)
  {
    if (e & decltype(e)(1)) f(x);

    if (e /= 2) [[likely]] x *= x; else [[unlikely]] return;
  }
}

template <auto X, std::size_t E>
inline constexpr auto pwrs{
  []<auto ...I>(std::index_sequence<I...>) noexcept
  {
    return std::array<decltype(X), E + 1>{pow(X, I)...};
  }(std::make_index_sequence<E + 1>())
};

template <auto X, std::size_t E>
inline constexpr auto pwrs2{
  []<auto ...I>(std::index_sequence<I...>) noexcept
  {
    return std::array<decltype(X), E + 1>{
      pow(X, pow(decltype(E)(2), I))...};
  }(std::make_index_sequence<E + 1>())
};

template <typename U, auto E>
inline constexpr auto maxaligns{
  []<auto ...I>(std::index_sequence<I...>) noexcept
  {
    return std::array<U, log<decltype(E)(2)>(E) + 1>{
      max_v<U> / pow(U(10), pow(decltype(E)(2), sizeof...(I) - 1 - I))...
    };
  }(std::make_index_sequence<log<decltype(E)(2)>(E) + 1>())
};

template <typename U, auto E>
inline constexpr auto minaligns{
  []<auto ...I>(std::index_sequence<I...>) noexcept
  {
    return std::array<U, log<decltype(E)(2)>(E) + 1>{
      min_v<U> / pow(U(10), pow(decltype(E)(2), sizeof...(I) - 1 - I))...
    };
  }(std::make_index_sequence<log<decltype(E)(2)>(E) + 1>())
};

template <typename T, typename U, auto E>
inline constexpr auto maxnorms{
  []<auto ...I>(std::index_sequence<I...>) noexcept
  {
    return std::array<U, log<decltype(E)(2)>(E) + 1>{
      U(max_v<T>) * pow(U(10), pow(decltype(E)(2), sizeof...(I) - 1 - I)) -
      (sizeof...(I) - 1 - I ? 0 : 5)...
    };
  }(std::make_index_sequence<log<decltype(E)(2)>(E) + 1>())
};

template <typename T, typename U, auto E>
inline constexpr auto minnorms{
  []<auto ...I>(std::index_sequence<I...>) noexcept
  {
    return std::array<U, log<decltype(E)(2)>(E) + 1>{
      U(min_v<T>) * pow(U(10), pow(decltype(E)(2), sizeof...(I) - 1 - I)) +
      (sizeof...(I) - 1 - I ? 0 : 5)...
    };
  }(std::make_index_sequence<log<decltype(E)(2)>(E) + 1>())
};

template <typename T>
constexpr void align(auto& ma, auto& ea, decltype(ma) mb,
  std::remove_reference_t<decltype(ea)> i) noexcept
{
  using U = std::remove_reference_t<decltype(ma)>;
  using F = std::remove_reference_t<decltype(ea)>;

  [&]() noexcept
  {
    if (intt::is_neg(ma))
      //for (; i && (ma >= ar::coeff<min_v<U> / 10>()); --i, --ea, ma *= T(10));
      for (;;)
      for (auto j(ar::coeff<logend<T>>());
        auto& m: minaligns<U, maxpow10e<T>()>)
      {
        if (auto const e(pwrs<F(2), logend<T>>[j]);
          (e <= i) && (ma >= m)) [[likely]]
          i -= e, ea -= e, ma *= pwrs2<U(10), logend<T>>[j];
        else if (!i || (ma < ar::coeff<U(min_v<U> / 10)>())) [[unlikely]]
          return;
        --j;
      }
    else
      //for (; i && (ma <= ar::coeff<max_v<U> / 10>()); --i, --ea, ma *= T(10));
      for (;;)
      for (auto j(ar::coeff<logend<T>>());
        auto& m: maxaligns<U, maxpow10e<T>()>)
      {
        if (auto const e(pwrs<F(2), logend<T>>[j]);
          (e <= i) && (ma <= m)) [[likely]]
          i -= e, ea -= e, ma *= pwrs2<U(10), logend<T>>[j];
        else if (!i || (ma > ar::coeff<U(max_v<U> / 10)>())) [[unlikely]]
          return;
        --j;
      }
  }();

  if (i) [&]() noexcept
    {
      for (;;)
      {
        auto j(ar::coeff<logend<T>>());

        do
        {
          if (auto const e(pwrs<F(2), logend<T>>[j]); e <= i) [[likely]]
            i -= e, mb /= pwrs2<U(10), logend<T>>[j];
          else if (!i || !mb) [[unlikely]] return;
        }
        while (j--);
      }
    }();
}

}

template <typename T, typename E>
  requires(detail::is_signed_v<T> && detail::is_signed_v<E>)
class dpp
{
public:
  using sig_t = T;

  static constexpr auto mmin{detail::min_v<T>};
  static constexpr auto mmax{detail::max_v<T>};

  using sig2_t = std::conditional_t<
      ar::bit_size_v<detail::double_t<T>> <= ar::bit_size_v<std::intmax_t>,
      std::intmax_t,
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

public:
  T m_; E e_;

public:
  dpp() = default;

  dpp(dpp const&) = default;
  dpp(dpp&&) = default;

  constexpr dpp(sig2_t m, exp2_t e) noexcept
  {
    using U = sig2_t;
    using F = exp2_t;

    using namespace detail;

    if (m < ar::coeff<U(mmin)>())
    {
      //for (++e; m < ar::coeff<U(10 * U(mmin) + 5)>(); ++e, m /= 10);
      [&]() noexcept
      {
        for (++e;;)
        for (auto i(ar::coeff<logend<T>>());
          auto& m0: minnorms<T, U, maxpow10e<T>()>)
        {
          if (m < m0) [[likely]]
            e += pwrs<F(2), logend<T>>[i],
            m /= pwrs2<U(10), logend<T>>[i];
          else if (m >= ar::coeff<U(10 * U(mmin) + 5)>()) [[unlikely]] return;

          --i;
        }
      }();

      m = (m - U(5)) / U(10);
    }
    else if (m > ar::coeff<U(mmax)>())
    {
      //for (++e; m > ar::coeff<U(10 * U(mmax) - 5)>(); ++e, m /= 10);
      [&]() noexcept
      {
        for (++e;;)
        for (auto i(ar::coeff<logend<T>>());
          auto& m0: maxnorms<T, U, maxpow10e<T>()>)
        {
          if (m > m0) [[likely]]
            e += pwrs<F(2), logend<T>>[i],
            m /= pwrs2<U(10), logend<T>>[i];
          else if (m <= ar::coeff<U(10 * U(mmax) - 5)>()) [[unlikely]] return;

          --i;
        }
      }();

      m = (m + U(5)) / U(10);
    }

    //
    if (e <= ar::coeff<F(emax)>()) [[likely]]
    {
      //while ((e <= ar::coeff<F(emin)>()) && m) ++e, m /= 10;
      if (e <= ar::coeff<F(emin)>())
        [&]() noexcept
        {
          for (;;)
          {
            auto i(ar::coeff<logend<T>>());

            do
            {
              if (auto const tmp(e + pwrs<F(2), logend<T>>[i]);
                tmp <= ar::coeff<F(emin + 1)>()) [[likely]]
                e = tmp, m /= pwrs2<U(10), logend<T>>[i];
              else if ((e > ar::coeff<F(emin)>()) || !m) [[unlikely]] return;
            }
            while (i--);
          }
        }();

      e_ = (m_ = m) ? E(e) : E{};
    }
    else [[unlikely]]
    {
      *this = nan;
    }
  }

  template <detail::integral U>
  constexpr dpp(U m, exp2_t e) noexcept
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

  constexpr dpp(detail::integral auto const m) noexcept: dpp(m, {}) { }

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
    {
      *this = nan;
    }
  }

  //
  constexpr dpp(direct_t, sig_t const m, exp_t const e) noexcept:
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
    return isnan(*this) || m_;
  }

  template <std::floating_point U>
  constexpr explicit (sizeof(U) != sizeof(m_)) operator U() const noexcept
  {
    if (isnan(*this)) [[unlikely]]
    {
      return NAN;
    }
    else [[likely]]
    {
      int const e2(
        std::ceil(int(e_) * 3.32192809488736234787031942948939017586483139f)
      );

      auto a(*this);

      e2 <= 0 ?
        detail::pow(dpp(direct, 2, {}), e2, [&](auto&& x) noexcept {a *= x;}):
        detail::pow(dpp(direct, 2, {}), e2, [&](auto&& x) noexcept {a /= x;});

      return std::ldexp(U(sig_t(a)), e2);
    }
  }

  template <detail::integral U>
  constexpr explicit operator U() const noexcept
  { // this function is unsafe, take a look at to_integral() for safety
    using F = exp2_t;

    if (intt::is_neg(e_))
    {
      auto m(m_);

      [&]() noexcept
      {
        F e(-F(e_)); // overflow prevention

        for (;;)
        {
          using namespace detail;

          auto i(ar::coeff<logend<T>>());

          do
          {
            if (auto const e0(pwrs<F(2), logend<T>>[i]);
              e0 <= e) [[likely]] e -= e0, m /= pwrs2<T(10), logend<T>>[i];
            else if (!e || !m) [[unlikely]] return;
          }
          while (i--);
        }
      }();

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
    return *this += ar::coeff<dpp{direct, 1, {}}>();
  }

  constexpr auto& operator--() noexcept
  {
    return *this -= ar::coeff<dpp{direct, 1, {}}>();
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

  constexpr dpp operator-() const noexcept
  {
    // we need to do it like this, as negating the sig can overflow
    if (isnan(*this)) [[unlikely]] return nan; else
      [[likely]] if (ar::coeff<mmin>() == m_) [[unlikely]]
        return dpp(ar::coeff<sig2_t(-sig2_t(mmin))>(), e_); else
        [[likely]] return dpp(direct, -m_, e_);
  }

  constexpr dpp operator+(dpp const& o) const noexcept
  {
    if (isnan(*this) || isnan(o)) [[unlikely]]
    {
      return nan;
    }
    else if (!m_) [[unlikely]]
    {
      return o;
    }
    else if (!o.m_) [[unlikely]]
    {
      return *this;
    }
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
    {
      return nan;
    }
    else if (!o.m_) [[unlikely]]
    {
      return *this;
    }
    else if (!m_) [[unlikely]]
    { // prevent overflow
      return ar::coeff<mmin>() == o.m_ ?
        dpp(ar::coeff<sig2_t(-sig2_t(mmin))>(), o.e_) :
        dpp(direct, -o.m_, o.e_);
    }
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
    {
      return nan;
    }
    else if (m_) [[likely]]
    {
      using U = sig2_t;
      using F = exp2_t;

      using namespace detail;

      constexpr auto e0(maxpow10e<T, F>());

      auto e(F(e_) - F(o.e_) - e0);
      auto m(ar::coeff<pow(U(10), e0)>() * U(m_));

      [&]() noexcept
      {
        if (intt::is_neg(m))
        {
          //for (; m >= ar::coeff<detail::min_v<U> / 10>(); m *= U(10), --e);
          for (;;)
          for (auto i(ar::coeff<logend<T>>());
            auto& m0: minaligns<U, maxpow10e<T>()>)
          {
            if (m >= m0) [[likely]]
              e -= pwrs<F(2), logend<T>>[i], m *= pwrs2<U(10), logend<T>>[i];
            else if (m < ar::coeff<U(min_v<U> / 10)>()) [[unlikely]] return;
            --i;
          }
        }
        else
        {
          //for (; m <= ar::coeff<detail::max_v<U> / 10>(); m *= U(10), --e);
          for (;;)
          for (auto i(ar::coeff<logend<T>>());
            auto& m0: maxaligns<U, maxpow10e<T>()>)
          {
            if (m <= m0) [[likely]]
              e -= pwrs<F(2), logend<T>>[i], m *= pwrs2<U(10), logend<T>>[i];
            else if (m > ar::coeff<U(max_v<U> / 10)>()) [[unlikely]] return;
            --i;
          }
        }
      }();

      return dpp(m / sig2_t(o.m_), e);
    }
    else [[unlikely]]
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
    else if (!m_ || !o.m_) [[unlikely]]
    {
      return m_ <=> o.m_;
    }
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
  return !intt::is_neg(a.exp()) || isnan(a) ? a : dpp<T, E>(direct, T(a), {});
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
  dpp<T, E> const c(direct, 5, -1);

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

  constexpr auto e0{detail::maxpow10e<U, F>()};

  if (isnan(a) || !a.m_) [[unlikely]] return nan; else
    [[likely]] return dpp<T, E>{
        ar::coeff<detail::pow(U(10), e0)>() / U(a.m_), F(-F(a.e_)) - e0
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
    typename T::exp2_t e{};

    for (bool dcp{}; end != i; ++i)
    {
      switch (*i)
      {
        [[likely]] case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          if (ar::coeff<T::emin>() == e) [[unlikely]] break; else [[likely]]
          {
            if (r >= ar::coeff<T::mmin / 10>()) [[likely]]
            {
              if (decltype(r) const t(10 * r), d(*i - '0');
                t >= ar::coeff<T::mmin>() + d) [[likely]]
              {
                r = t - d;
                e -= dcp;

                continue;
              }
            }
          }

          break;

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
    return {
        neg ? r : ar::coeff<T::mmin>() == r ? ar::coeff<T::mmax>() : -r,
        e
      };
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
  {
    return {"nan", 3};
  }
  else [[likely]]
  {
    auto m(a.sig());
    F e;

    if (m) [[likely]]
    {
      if (intt::is_neg(e = a.exp()))
      { // for (; !(m % 10); ++e, m /= 10);
        [&]() noexcept
        {
          using namespace detail;

          for (;;)
          {
            auto i(ar::coeff<logend<T>>());

            do
            { // slash zeros
              if (auto& m0(pwrs2<T(10), logend<T>>[i]);
                !(m % m0)) [[likely]] e += pwrs<F(2), logend<T>>[i], m /= m0;
              else if (m % T(10)) [[unlikely]] return;
            }
            while (i--);
          }
        }();
      }
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
  return to_decimal<d ## ID>(std::initializer_list<char>{c...});\
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

    if (dpp::isnan(a)) [[unlikely]]
    { // unique nan
      m = {};
    }
    else if ((m = a.sig())) [[likely]]
    { // unique everything
      // for (; !(m % 10); ++e, m /= 10); // slash zeros
      [&]() noexcept
      {
        using namespace dpp::detail;

        for (;;)
        {
          auto i(ar::coeff<logend<T>>());

          do
          { // slash zeros
            if (auto& m0(pwrs2<T(10), logend<T>>[i]);
              !(m % m0)) [[likely]]
              e += pwrs<F(2), logend<T>>[i], m /= m0;
            else if (m % T(10)) [[unlikely]] return;
          }
          while (i--);
        }
      }();
    }
    else [[unlikely]]
    { // unique zero
      e = {};
    }

    //
    auto const s(std::hash<decltype(m)>()(m));

    return s ^ (std::hash<F>()(e) + intt::consts::ISR + (s << 6) + (s >> 2));
  }
};

}

#endif // DPP_HPP
