#ifndef DPP_HPP
# define DPP_HPP
# pragma once

#include <climits>
#include <cmath> // std::pow
#include <cstdint>

#include <functional> // std::hash

#include <iterator>

#include <optional>

#include <ostream>

#include <string>

#include <type_traits>

namespace dpp
{

using direct = struct {};
using nan = struct {};
using unpack = struct {};

namespace detail
{

template <typename U>
constexpr static auto bit_size_v(CHAR_BIT * sizeof(U));

template <typename U>
constexpr static auto is_integral_v(std::is_integral_v<U> ||
  std::is_same_v<U, __int128>
);

template <typename U>
constexpr static auto is_signed_v(std::is_signed_v<U> ||
  std::is_same_v<U, __int128>
);

template <typename T, int B>
constexpr T pow(int e) noexcept
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

constexpr int log10(auto const x, int const e = 0u) noexcept
{
  return pow<std::remove_cv_t<decltype(x)>, 10>(e) > x ? e : log10(x, e + 1);
}

template <auto a, typename B>
constexpr B selectsign(B const b) noexcept
{
  if constexpr (is_signed_v<decltype(a)> && is_signed_v<B>)
  {
    return b < 0 ? -a : a;
  }
  else
  {
    return a;
  }
}

// ae and be are minimal, cannot be reduced further, ae >= be, maximize be.
template <typename T>
constexpr void shift(T const am, int const ae, T& bm, int& be) noexcept
{
  if (am)
  {
    for (auto const c(detail::selectsign<5>(bm)); bm && (ae != be++);
      bm = (bm + c) / 10);

    be = ae;
  }
}

}

template <unsigned M>
class dpp
{
public:
  using exp_type = std::conditional_t<M == 16, std::int8_t, std::int16_t>;

  enum
  {
    emin = std::numeric_limits<exp_type>::min(),
    emax = std::numeric_limits<exp_type>::max()
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
    mmin = std::numeric_limits<mantissa_type>::min(),
    mmax = std::numeric_limits<mantissa_type>::max()
  };

  struct value_type
  {
    mantissa_type m;
    exp_type e;
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
        __int128_t,
        void
      >
    >
  >;

  struct value_type v_{};

public:
  constexpr dpp() = default;

  constexpr dpp(dpp const&) = default;
  constexpr dpp(dpp&&) = default;

  template <typename U>
  constexpr dpp(U m, int e) noexcept requires(detail::is_integral_v<U>)
  {
    constexpr auto umin(detail::is_signed_v<U> ?
      U(1) << (detail::bit_size_v<U> - 1) : U{});
    constexpr auto umax(detail::is_signed_v<U> ? -(umin + 1) : ~U{});

    // slash m, if necessary
    if constexpr (detail::is_signed_v<U> && (detail::bit_size_v<U> > M))
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

    if constexpr ((detail::is_signed_v<U> && (detail::bit_size_v<U> > M)) ||
      (std::is_unsigned_v<U> && (detail::bit_size_v<U> >= M)))
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
  constexpr dpp(U const m) noexcept requires(detail::is_integral_v<U>):
    dpp(std::conditional_t<
          detail::bit_size_v<U> < detail::bit_size_v<mantissa_type>,
          mantissa_type,
          U
        >(m), 0
    )
  {
  }

  template <unsigned A>
  constexpr dpp(dpp<A> const o) noexcept: dpp(o.mantissa(), o.exponent()) { }

  template <typename U>
  dpp(U f) noexcept requires(std::is_floating_point_v<U>)
  {
    if (std::isfinite(f))
    {
      int e{};

      // eliminate the fractional part
      for (; std::trunc(f) != f; f *= 10, --e);

      // slash f, if necessary
      for (constexpr long double
        min(std::numeric_limits<std::intmax_t>::min()),
        max(std::numeric_limits<std::intmax_t>::max());
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

  constexpr dpp(auto const o, unpack) noexcept: dpp(o.m, o.e) { }

  //
  constexpr explicit operator bool() const noexcept
  {
    return isnan(*this) || v_.m;
  }

  template <typename T>
  explicit (sizeof(T) != sizeof(v_)) operator T() const noexcept
    requires(std::is_floating_point_v<T>)
  {
    if (isnan(*this))
    {
      return NAN;
    }
    else if (auto m(v_.m); m)
    {
      int e(v_.e);

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
    if (int e(v_.e); e > 0)
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
    return isnan(*this) ? dpp{nan{}} :
      mmin == v_.m ? dpp(-v_.m, v_.e) : dpp(-v_.m, v_.e, direct{});
  }

  constexpr dpp operator+(dpp const o) const noexcept
  {
    if (isnan(*this) || isnan(o))
    {
      return nan{};
    }
    else
    {
      auto ma(v_.m), mb(o.v_.m);

      if (int ea(v_.e), eb(o.v_.e); ea < eb)
      {
        detail::shift(mb, eb, ma, ea);

        return {doubled_t(ma) + mb, ea};
      }
      else
      {
        detail::shift(ma, ea, mb, eb);

        return {doubled_t(ma) + mb, eb};
      }
    }
  }

  constexpr dpp operator-(dpp const o) const noexcept
  {
    if (isnan(*this) || isnan(o))
    {
      return nan{};
    }
    else
    {
      auto ma(v_.m), mb(o.v_.m);

      if (int ea(v_.e), eb(o.v_.e); ea < eb)
      {
        detail::shift(mb, eb, ma, ea);

        return {doubled_t(ma) - mb, ea};
      }
      else
      {
        detail::shift(ma, ea, mb, eb);

        return {doubled_t(ma) - mb, eb};
      }
    }
  }

  constexpr dpp operator*(dpp const o) const noexcept
  {
    return isnan(*this) || isnan(o) ? nan{} :
      dpp<M>{doubled_t(v_.m) * o.v_.m, v_.e + o.v_.e};
  }

  constexpr dpp operator/(dpp const o) const noexcept
  {
    if (isnan(*this) || isnan(o) || !o.v_.m) // guard against division by 0
    {
      return nan{};
    }
    else if (v_.m) // guard against division by 0
    {
      enum : doubled_t
      {
        rmin = doubled_t(1) << (detail::bit_size_v<doubled_t> - 1),
        rmax = -(rmin + 1)
      };

      // dp is the exponent, that generates the maximal power of 10,
      // that fits into doubled_t
      // 10^dp > rmax, hence 10^(dp - 1) <= rmax
      enum { dp = detail::log10((long double)(rmax)) - 1 };

      int e(v_.e - o.v_.e - dp);

      // we want an approximation to a.v_.m * (10^dp / b.v_.m)
      auto q(detail::pow<doubled_t, 10>(dp) / o.v_.m);

      // fit q * v_.m into doubled_t, q * v_.m <= rmax, q * v_.m >= rmin
      if (auto const am(v_.m < 0 ? -v_.m : v_.m); q < 0)
      {
        for (auto const a(rmin / am); q < a; q /= 10, ++e);
      }
      else
      {
        for (auto const a(rmax / am); q > a; q /= 10, ++e);
      }

      return {q * v_.m, e};
    }
    else
    {
      return {};
    }
  }

  //
  constexpr bool operator==(dpp const o) const noexcept
  {
    return !isnan(*this) && !isnan(o) && (v_.e == o.v_.e) && (v_.m == o.v_.m);
  }

  constexpr bool operator<(dpp const o) const noexcept
  {
    if (isnan(*this) || isnan(o))
    {
      return false;
    }
    else
    {
      auto ma(v_.m), mb(o.v_.m);

      if (int ea(v_.e), eb(o.v_.e); ea < eb)
      {
        detail::shift(mb, eb, ma, ea);
      }
      else
      {
        detail::shift(ma, ea, mb, eb);
      }

      return ma < mb;
    }
  }

  //
  static constexpr auto min() noexcept { return dpp{mmin, emax}; }
  static constexpr auto max() noexcept { return dpp{mmax, emax}; }

  //
  constexpr auto exponent() const noexcept { return v_.e; }
  constexpr auto mantissa() const noexcept { return v_.m; }

  constexpr auto packed() const noexcept { return v_; }
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
  return (a > b) - (a < b);
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
DPP_LEFT_CONVERSION(<)
DPP_LEFT_CONVERSION(!=)
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
DPP_RIGHT_CONVERSION(<)
DPP_RIGHT_CONVERSION(!=)
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
  if (int e(a.exponent()); !isnan(a) && (e < 0))
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
template <typename T, typename It>
constexpr T to_decimal(It i, It const end) noexcept
{
  if (i == end)
  {
    return nan{};
  }
  else
  {
    constexpr auto rmax(std::numeric_limits<std::intmax_t>::max());
    constexpr auto rmin(std::numeric_limits<std::intmax_t>::min());

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

    typename T::mantissa_type r{};
    int e{};

    auto const scandigit([&]<T::mantissa_type DE>(char const c) noexcept
      {
        if (positive)
        {
          if (r <= rmax / 10)
          {
            r *= 10;

            if (auto const d(c - '0'); r <= rmax - d)
            {
              r += d;
              if constexpr (DE) e += DE;

              return false;
            }
          }
        }
        else
        {
          if (r >= rmin / 10)
          {
            r *= 10;

            if (auto const d(c - '0'); r >= rmin + d)
            {
              r -= d;
              if constexpr (DE) e += DE;

              return false;
            }
          }
        }

        return true;
      }
    );

    for (; i != end; i = std::next(i))
    {
      switch (*i)
      {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          if (scandigit.template operator()<0>(*i)) break; else continue;

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

    for (; i != end; i = std::next(i))
    {
      switch (*i)
      {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          if (scandigit.template operator()<-1>(*i)) break; else continue;

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
  if (isnan(p))
  {
    return {};
  }
  else
  {
    T m(p.mantissa());

    if (int e(p.exponent()); e <= 0)
    {
      for (; m && e++; m /= 10);

      return m;
    }
    else
    {
      do
      {
        if ((m >= std::numeric_limits<T>::min() / 10) &&
          (m <= std::numeric_limits<T>::max() / 10))
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
      int e(t.exponent());

      for (; !(m % 10); m /= 10, ++e);

      r.append(std::to_string(m)).append(e, '0');
    }
    else
    {
      p.mantissa() < 0 ? r.append("-0", 2) : r.append(1, '0');
    }

    auto m(p.mantissa());

    if (int e(p.exponent()); (e < 0) && m)
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
  auto operator()(dpp::dpp<M> const a) const noexcept
  {
    auto const hash_combine([](auto&& ...v) noexcept
      requires(bool(sizeof...(v)))
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
