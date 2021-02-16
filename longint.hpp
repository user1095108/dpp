#ifndef LONGINT_HPP
# define LONGINT_HPP
# pragma once

#include <cassert>
#include <climits>

#include <array>

#include <limits>

#include <ostream>

#include <sstream>

#include <utility>

#include <type_traits>

namespace longint
{

template <typename T, unsigned N>
class longint
{
static_assert(std::is_unsigned_v<T>);
static_assert(N > 0);

public:
using value_type = std::array<T, N>;

value_type v_{};

public:
enum : unsigned { bits = N * sizeof(T) * CHAR_BIT };
enum : unsigned { bits_e = sizeof(T) * CHAR_BIT };

enum : T { max_e = std::numeric_limits<T>::max() };

enum : unsigned { size = N };

constexpr longint() noexcept = default;

constexpr longint(decltype(v_) const& v) noexcept :
  v_(v)
{
}

template <typename U,
  std::enable_if_t<
    std::is_integral_v<U>,
    int
  > = 0
>
constexpr longint(U const v) noexcept
{
  auto const convert([&]<std::size_t ...I>(std::index_sequence<I...>) noexcept
    {
      if constexpr (std::is_signed_v<U>)
      {
        (
          (
            v_[I] = I * bits_e < sizeof(U) * CHAR_BIT ?
              v >> I * bits_e :
              v >= 0 ? T{} : ~T{}
          ),
          ...
        );
      }
      else
      {
        (
          (
            v_[I] = I * bits_e < sizeof(U) * CHAR_BIT ?
              v >> I * bits_e :
              T{}
          ),
          ...
        );
      }
    }
  );

  convert(std::make_index_sequence<bits / bits_e>());
}

constexpr longint(longint const&) = default;
constexpr longint(longint&&) = default;

//
constexpr longint& operator=(longint const&) = default;
constexpr longint& operator=(longint&&) = default;

//
constexpr explicit operator bool() const noexcept
{
  return any(*this);
}

template <typename U,
   std::enable_if_t<
    std::is_integral_v<U> && std::is_signed_v<U>,
    int
  > = 0
>
constexpr explicit operator U() const noexcept
{
  auto const convert([&]<std::size_t ...I>(std::index_sequence<I...>) noexcept
    {
      if constexpr (sizeof...(I))
      {
        return (
          (U(v_[I]) << I * bits_e) |
          ...
        );
      }
      else
      {
        return v_[0];
      }
    }
  );

  return convert(std::make_index_sequence<(sizeof(U) * CHAR_BIT) / bits_e>());
}

// assignment
template <typename A>
constexpr auto& operator+=(A&& a) noexcept
{
  return *this = *this + std::forward<A>(a);
}

template <typename A>
constexpr auto& operator-=(A&& a) noexcept
{
  return *this = *this - std::forward<A>(a);
}

template <typename A>
constexpr auto& operator*=(A&& a) noexcept
{
  return *this = *this * std::forward<A>(a);
}

template <typename A>
constexpr auto& operator/=(A&& a) noexcept
{
  return *this = *this / std::forward<A>(a);
}

// increment, decrement
constexpr auto& operator++() noexcept { return *this += 1; }
constexpr auto& operator--() noexcept { return *this -= 1; }

//
constexpr auto& operator<<=(unsigned i) noexcept
{
  return *this = *this << i;
}

constexpr auto& operator>>=(unsigned i) noexcept
{
  return *this = *this >> i;
}

// member access
constexpr auto operator[](unsigned const i) noexcept
{
  return v_[i];
}

constexpr auto operator[](unsigned const i) const noexcept
{
  return v_[i];
}

//
template <typename A, unsigned B>
friend constexpr bool operator==(longint<A, B> const&,
  longint<A, B> const&) noexcept;

template <typename A, unsigned B, typename C, unsigned D>
friend constexpr auto operator+(longint<A, B> const&,
  longint<C, D> const&) noexcept;

template <typename A, unsigned B, typename C, unsigned D>
friend constexpr auto operator-(longint<A, B> const&,
  longint<C, D> const&) noexcept;

template <typename A, unsigned B>
friend constexpr auto operator<<(longint<A, B> const&, unsigned) noexcept;
template <typename A, unsigned B>
friend constexpr auto operator>>(longint<A, B> const&, unsigned) noexcept;
};

//arithmetic//////////////////////////////////////////////////////////////////
template <typename T, unsigned N>
constexpr auto& operator+(longint<T, N> const& a) noexcept
{
  return a;
}

template <typename T, unsigned N>
constexpr auto operator-(longint<T, N> const& a) noexcept
{
  return ~a + 1;
}

template <typename T, unsigned N>
constexpr auto operator~(longint<T, N> const& a) noexcept
{
  auto const neg([&]<std::size_t ...I>(
    std::index_sequence<I...>) noexcept
    {
      return typename longint<T, N>::value_type{T(~a[I])...};
    }
  );

  return longint<T, N>(neg(std::make_index_sequence<N>()));
}

template <typename T, unsigned N>
constexpr auto operator+(longint<T, N> const& a,
  longint<T, N> const& b) noexcept
{
  using r_t = longint<T, N>;

  auto const add([&]<std::size_t ...I>(
    std::index_sequence<I...>) noexcept
    {
      longint<T, N> r;

      bool carry{};

      (
        (
          r.v_[I] = a[I] + b[I] + carry,
          carry = a[I] > r_t::max_e - b[I] - carry
        ),
        ...
      );

      return r;
    }
  );

  return add(std::make_index_sequence<N>());
}

template <typename T, unsigned N>
constexpr auto operator-(longint<T, N> const& a,
  longint<T, N> const& b) noexcept
{
  return a + (-b);
}

template <typename T, unsigned N>
constexpr auto operator*(longint<T, N> const& a,
  longint<T, N> const& b) noexcept
{
  using r_t = longint<T, N>;

  auto const mul([&]<std::size_t ...I>(std::index_sequence<I...>) noexcept
    {
      auto const mul([&]<std::size_t I0>() noexcept
        {
          return (
            (
              (I0 + I) * r_t::bits_e <
                N * r_t::bits_e - sizeof(a[I0] * b[I]) * CHAR_BIT ?
                r_t(a[I0] * b[I]) << (I0 + I) * r_t::bits_e :
                r_t{}
            ) +
            ...
          );
        }
      );

      return (mul.template operator()<I>() + ...);
    }
  );

  return mul(std::make_index_sequence<N>());
}

template <typename T, unsigned N>
constexpr auto operator/(longint<T, N> a, longint<T, N> b) noexcept
{
  using r_t = longint<T, N>;

  if (negative(b))
  {
    a = -a;
    b = -b;
  }

  r_t q;
  r_t r(a);

  std::cout << int(a) << " " << int(b) << " " << int(q) << " " << int(r) << " : " << to_raw(r) << std::endl;

  if (negative(a))
  {
    while (r >= b)
    {
      --q;
      r += b;
    }
  }
  else
  {
    while (r >= b)
    {
      ++q;
      r -= b;
    }
  }

  return q;
}

template <typename A, unsigned B>
constexpr auto operator%(longint<A, B> const& a,
  longint<A, B> const& b) noexcept
{
  return a - (a / b) * b;
}

// conversions
template <typename A, unsigned B, typename U>
constexpr auto operator+(longint<A, B> const& a, U const b) noexcept
{
  static_assert(std::is_arithmetic_v<U>);
  return a + longint<A, B>(b);
}

template <typename A, unsigned B, typename U>
constexpr auto operator-(longint<A, B> const& a, U const b) noexcept
{
  static_assert(std::is_arithmetic_v<U>);
  return a - longint<A, B>(b);
}

template <typename A, unsigned B, typename U>
constexpr auto operator*(longint<A, B> const& a, U const b) noexcept
{
  static_assert(std::is_arithmetic_v<U>);
  return a * longint<A, B>(b);
}

template <typename A, unsigned B, typename U>
constexpr auto operator/(longint<A, B> const& a, U const b) noexcept
{
  static_assert(std::is_arithmetic_v<U>);
  return a / longint<A, B>(b);
}

template <typename A, unsigned B, typename U>
constexpr auto operator%(longint<A, B> const& a, U const b) noexcept
{
  static_assert(std::is_arithmetic_v<U>);
  return a % longint<A, B>(b);
}

// conversions
template <typename A, unsigned B, typename U>
constexpr auto operator+(U const a, longint<A, B> const& b) noexcept
{
  static_assert(std::is_arithmetic_v<U>);
  return longint<A, B>(a) + b;
}

template <typename A, unsigned B, typename U>
constexpr auto operator-(U const a, longint<A, B> const& b) noexcept
{
  static_assert(std::is_arithmetic_v<U>);
  return longint<A, B>(a) - b;
}

template <typename A, unsigned B, typename U>
constexpr auto operator*(U const a, longint<A, B> const& b) noexcept
{
  static_assert(std::is_arithmetic_v<U>);
  return longint<A, B>(a) * b;
}

template <typename A, unsigned B, typename U>
constexpr auto operator/(U const a, longint<A, B> const& b) noexcept
{
  static_assert(std::is_arithmetic_v<U>);
  return longint<A, B>(a) / b;
}

template <typename A, unsigned B, typename U>
constexpr auto operator%(U const a, longint<A, B> const& b) noexcept
{
  static_assert(std::is_arithmetic_v<U>);
  return longint<A, B>(a) % b;
}

//
template <typename T, unsigned N>
constexpr auto operator<<(longint<T, N> const& a, unsigned M) noexcept
{
  auto r(a);

  auto const shl([&]<std::size_t ...I>(unsigned const e,
    std::index_sequence<I...>) noexcept
    {
      (
        (
          r.v_[N - 1 - I] = 
            (r[N - 1 - I] << e) |
            (r[N - 1 - I - 1] >> (longint<T, N>::bits_e - e))
        ),
        ...
      );

      r.v_[0] <<= e;
    }
  );

  while (M >= longint<T, N>::bits_e)
  {
    shl.template operator()(longint<T, N>::bits_e,
      std::make_index_sequence<N - 1>());
    M -= longint<T, N>::bits_e;
  }

  shl.template operator()(M % longint<T, N>::bits_e,
    std::make_index_sequence<N - 1>());

  return r;
}

template <typename T, unsigned N>
constexpr auto operator>>(longint<T, N> const& a, unsigned M) noexcept
{
  auto const neg(negative(a));

  auto r(a);

  auto const shr([&]<std::size_t ...I>(unsigned const e,
    std::index_sequence<I...>) noexcept
    {
      (
        (
          r.v_[I] =
            (r[I] >> e) |
            (r[I + 1] << (longint<T, N>::bits_e - e))
        ),
        ...
      );

      r.v_[N - 1] = (r.v_[N - 1] >> e) |
        (neg ? T(~T{}) << (longint<T, N>::bits_e - e) : T{});
    }
  );

  while (M >= longint<T, N>::bits_e)
  {
    shr.template operator()(longint<T, N>::bits_e,
      std::make_index_sequence<N - 1>());
    M -= longint<T, N>::bits_e;
  }

  shr.template operator()(M % longint<T, N>::bits_e,
    std::make_index_sequence<N - 1>());

  return r;
}

//comparison//////////////////////////////////////////////////////////////////
template <typename A, unsigned B>
constexpr bool operator==(longint<A, B> const& a,
  longint<A, B> const& b) noexcept
{
  return a.v_ == b.v_;
}

template <typename A, unsigned B>
constexpr auto operator!=(longint<A, B> const& a,
  longint<A, B> const& b) noexcept
{
  return !(a == b);
}

//
template <typename A, unsigned B>
constexpr bool operator<(longint<A, B> const& a,
  longint<A, B> const& b) noexcept
{
  return negative(a - b);
}

template <typename A, unsigned B>
constexpr auto operator>(longint<A, B> const& a,
  longint<A, B> const& b) noexcept
{
  return b < a;
}

template <typename A, unsigned B>
constexpr auto operator<=(longint<A, B> const& a,
  longint<A, B> const& b) noexcept
{
  return !(b < a);
}

template <typename A, unsigned B>
constexpr auto operator>=(longint<A, B> const& a,
  longint<A, B> const& b) noexcept
{
  return !(a < b);
}

#if __cplusplus > 201703L
template <typename A, unsigned B>
constexpr auto operator<=>(longint<A, B> const& a,
  longint<A, B> const& b) noexcept
{
  return (a > b) - (a < b);
}
#endif

// conversions
template <typename A, unsigned B, typename U>
constexpr auto operator==(longint<A, B> const& a, U const& b) noexcept
{
  return a == longint<A, B>(b);
}

template <typename A, unsigned B, typename U>
constexpr auto operator!=(longint<A, B> const& a, U const& b) noexcept
{
  return a != longint<A, B>(b);
}

template <typename A, unsigned B, typename U>
constexpr auto operator<(longint<A, B> const& a, U const& b) noexcept
{
  return a < longint<A, B>(b);
}

template <typename A, unsigned B, typename U>
constexpr auto operator>(longint<A, B> const& a, U const& b) noexcept
{
  return a > longint<A, B>(b);
}

template <typename A, unsigned B, typename U>
constexpr auto operator<=(longint<A, B> const& a, U const& b) noexcept
{
  return a <= longint<A, B>(b);
}

template <typename A, unsigned B, typename U>
constexpr auto operator>=(longint<A, B> const& a, U const& b) noexcept
{
  return a >= longint<A, B>(b);
}

#if __cplusplus > 201703L
template <typename A, unsigned B, typename U>
constexpr auto operator<=>(longint<A, B> const& a, U const& b) noexcept
{
  return (a > b) - (a < b);
}
#endif

// conversions
template <typename A, unsigned B, typename U>
constexpr auto operator==(U const& a, longint<A, B> const& b) noexcept
{
  return longint<A, B>(a) == b;
}

template <typename A, unsigned B, typename U>
constexpr auto operator!=(U const& a, longint<A, B> const& b) noexcept
{
  return longint<A, B>(a) != b;
}

template <typename A, unsigned B, typename U>
constexpr auto operator<(U const& a, longint<A, B> const& b) noexcept
{
  return longint<A, B>(a) < b;
}

template <typename A, unsigned B, typename U>
constexpr auto operator>(U const& a, longint<A, B> const& b) noexcept
{
  return longint<A, B>(a) > b;
}

template <typename A, unsigned B, typename U>
constexpr auto operator<=(U const& a, longint<A, B> const& b) noexcept
{
  return longint<A, B>(a) <= b;
}

template <typename A, unsigned B, typename U>
constexpr auto operator>=(U const& a, longint<A, B> const& b) noexcept
{
  return longint<A, B>(a) >= b;
}

//misc////////////////////////////////////////////////////////////////////////
template <typename A, unsigned B>
constexpr bool any(longint<A, B> const& a) noexcept
{
  auto const any([&]<std::size_t ...I>(std::index_sequence<I...>) noexcept
    {
      return (a[I] || ...);
    }
  );

  return any(std::make_index_sequence<B>());
}

template <typename A, unsigned B>
constexpr bool negative(longint<A, B> const& a) noexcept
{
//std::cout << "lol: " << a[B - 1] << std::endl;
  return a[B - 1] >> (longint<A, B>::bits_e - 1);
}

//
template <typename T, unsigned N>
auto to_raw(longint<T, N> const& a) noexcept
{
  std::stringstream ss;

  ss << std::hex;

  for (unsigned i(N - 1); i; --i)
  {
    ss << a[i] << " ";
  }

  ss << a[0];

  return ss.str();
}

template <typename T, unsigned N>
std::string to_string(longint<T, N> a)
{
  auto const neg(negative(a));

  std::string r;

  do
  {
    std::int8_t const d(a % 10);

    r.insert(0, 1, '0' + (neg ? -d : d));

    a /= 10;
  }
  while (a);

  if (neg)
  {
    r.insert(0, 1, '-');
  }

  return r;
}

template <typename A, unsigned B>
inline auto& operator<<(std::ostream& os, longint<A, B> const& p)
{
  return os << to_string(p);
}

}

namespace std
{

template <typename T, unsigned N>
struct hash<longint::longint<T, N>>
{
  auto operator()(longint::longint<T, N> const& a) const noexcept
  {
    auto const combine([&]<std::size_t ...I>(
      std::index_sequence<I...>) noexcept
      {
        std::size_t seed{672807365};

        (
          (
            seed ^= hash<T>()(a[I]) + 0x9e3779b9 + (seed << 6) + (seed >> 2)
          ),
          ...
        );

        return seed;
      }
    );

    return combine(std::make_index_sequence<N>());
  }
};

}

#endif // LONGINT_HPP
