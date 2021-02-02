#ifndef LONGINT_HPP
# define LONGINT_HPP
# pragma once

#include <array>

#include <limits>

#include <ostream>

#include <utility>

#include <type_traits>

namespace dpp
{

namespace detail::longint
{

template <unsigned B, typename T>
constexpr T pow(unsigned e) noexcept
{
  if (e)
  {
    T x(B), y(1);

    while (1 != e)
    {
      if (e % 2)
      {
        //--e;
        y *= x;
      }

      x *= x;
      e /= 2;
    }

    return x * y;
  }
  else
  {
    return T(1);
  }
}

constexpr unsigned log2(std::uintmax_t const x, unsigned e = 0u) noexcept
{
  return pow<2, std::uintmax_t>(e) >= x ? e : log2(x, e + 1);
}

}

template <typename T, unsigned N>
class longint
{
static_assert(std::is_unsigned_v<T>);
static_assert(N > 0);

public:
using value_type = std::array<T, N>;

value_type v_;

public:
enum : T { max_e = std::numeric_limits<T>::max() };

enum : unsigned { bits_e = detail::longint::log2(max_e) };

constexpr longint() noexcept :
  v_{}
{
}

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
constexpr longint(U v) noexcept
{
  unsigned i{};

  do
  {
    v_[i++] = v & max_e;
    v /= (max_e + 1);
  }
  while (v);
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

constexpr auto& operator[](unsigned const i) noexcept
{
  return v_[i];
}

constexpr auto& operator[](unsigned const i) const noexcept
{
  return v_[i];
}

//
template <typename U, unsigned M>
constexpr auto& operator+=(longint<U, M> const& a) noexcept
{
  return *this = *this + a;
}

template <typename U, unsigned M>
constexpr auto& operator-=(longint<U, M> const a) noexcept
{
  return *this = *this - a;
}

template <typename U, unsigned M>
constexpr auto& operator*=(longint<U, M> const a) noexcept
{
  return *this = *this * a;
}

//
template <typename U, unsigned O, unsigned P>
friend constexpr bool operator==(longint<U, O> const&,
  longint<U, P> const&) noexcept;
};

//misc////////////////////////////////////////////////////////////////////////
template <typename T, unsigned N>
constexpr bool any(longint<T, N> const& a) noexcept
{
  auto const any([&]<std::size_t ...I>(
    std::index_sequence<I...>) noexcept
    {
      return (a[I] || ...);
    }
  );

  return any(std::make_index_sequence<N>(a));
}

//logic///////////////////////////////////////////////////////////////////////
template <typename T, unsigned N>
constexpr auto operator!(longint<T, N> const& a) noexcept
{
  return !any(a);
}

//arithmetic//////////////////////////////////////////////////////////////////
template <typename T, unsigned N>
constexpr auto& operator+(longint<T, N> const& a) noexcept
{
  return a;
}

template <typename T, unsigned N>
constexpr auto operator-(longint<T, N> const& a) noexcept
{
  return ~a + longint<T, N>(1);
}

template <typename T, unsigned M, unsigned N>
constexpr auto operator+(longint<T, M> const& a,
  longint<T, N> const& b) noexcept
{
  longint<T, std::max(M, N)> r;

  auto const add([&]<std::size_t ...I>(
    std::index_sequence<I...>) noexcept
    {
      bool carry{};

      (
        (
          r[I] = (I < M ? a[I] : T{}) + (I < N ? b[I] : T{}) + carry,
          carry = (r[I] < a[I]) || (r[I] < b[I]) || (r[I] < carry)
        ),
        ...
      );
    }
  );

  return add(std::make_index_sequence<std::max(M, N)>()), r;
}

template <typename T, unsigned M, unsigned N>
constexpr auto operator-(longint<T, M> const& a,
  longint<T, N> const& b) noexcept
{
  return a + (-b);
}

template <typename T, unsigned M, unsigned N>
constexpr auto operator*(longint<T, M> const& a,
  longint<T, N> const& b) noexcept
{
  using r_t = longint<T, std::max(M, N)>;

  auto const mul([&]<std::size_t ...I, std::size_t ...J>(
    std::index_sequence<I...>, std::index_sequence<J...>) noexcept
    {
      return ((r_t(a[I] * b[J]) << ((I + J) * r_t::bits_e)) + ...);
    }
  );

  return mul(std::make_index_sequence<M>(), std::make_index_sequence<N>());
}

template <typename T, unsigned N>
constexpr auto operator~(longint<T, N> const& a) noexcept
{
  auto const negate([&]<std::size_t ...I>(
    std::index_sequence<I...>) noexcept -> longint<T, N>
    {
      return typename longint<T, N>::value_type{T(~a[I])...};
    }
  );

  return negate(std::make_index_sequence<N>());
}

template <typename T, unsigned N>
constexpr auto operator<<(longint<T, N> const& a, unsigned const M) noexcept
{
  auto r(a);

  auto const shl([&]<std::size_t ...I>(std::index_sequence<I...>) noexcept
    {
      (
        (
          r[N - 1 - I] = 
            (r[N - 1 - I] << 1) |
            (r[N - 1 - I - 1] >> (longint<T, N>::bits_e - 1))
        ),
        ...
      );

      r[0] <<= 1;
    }
  );

  for (unsigned i{}; M != i; ++i)
  {
    shl(std::make_index_sequence<N - 1>());
  }

  return r;
}

template <typename T, unsigned N>
constexpr auto operator>>(longint<T, N> const& a, unsigned const M) noexcept
{
  auto r(a);

  auto const shr([&]<std::size_t ...I>(std::index_sequence<I...>) noexcept
    {
      (
        (
          r[I] = 
            (r[I] >> 1) |
            (r[I + 1] & ((longint<T, N>::max_e >> 1) + 1))
        ),
        ...
      );

      r[N - 1] >>= 1;
    }
  );

  for (unsigned i{}; M != i; ++i)
  {
    shr(std::make_index_sequence<N - 1>());
  }

  return r;
}

//comparison//////////////////////////////////////////////////////////////////
template <typename T, unsigned M, unsigned N>
constexpr bool operator==(longint<T, M> const& a,
  longint<T, N> const& b) noexcept
{
  return a.v_ == b.v_;
}

template <typename T, unsigned M, unsigned N>
constexpr bool operator!=(longint<T, M> const& a,
  longint<T, N> const& b) noexcept
{
  return !(a == b);
}

//
template <typename T, unsigned M, unsigned N>
constexpr bool operator<(longint<T, M> const& a,
  longint<T, N> const& b) noexcept
{
  auto const r(a - b);

  return r[std::max(M, N) - 1] & ((decltype(r)::max_e >> 1) + 1);
}

template <typename T, unsigned M, unsigned N>
constexpr auto operator>(longint<T, M> const& a,
  longint<T, N> const& b) noexcept
{
  return b < a;
}

template <typename T, unsigned M, unsigned N>
constexpr auto operator<=(longint<T, M> const& a,
  longint<T, N> const& b) noexcept
{
  return !(b < a);
}

template <typename T, unsigned M, unsigned N>
constexpr auto operator>=(longint<T, M> const& a,
  longint<T, N> const& b) noexcept
{
  return !(a < b);
}

#if __cplusplus > 201703L
template <typename T, unsigned M, unsigned N>
constexpr auto operator<=>(longint<T, M> const& a,
  longint<T, N> const& b) noexcept
{
  return (a > b) - (a < b);
}
#endif

//
template <typename T, unsigned N, typename U>
constexpr auto operator==(longint<T, N> const& a, U const b) noexcept
{
  return a == longint<T, N>(b);
}

template <typename T, unsigned N, typename U>
constexpr auto operator!=(longint<T, N> const& a, U const b) noexcept
{
  return a != longint<T, N>(b);
}

template <typename T, unsigned N, typename U>
constexpr auto operator<(longint<T, N> const& a, U const b) noexcept
{
  return a < longint<T, N>(b);
}

template <typename T, unsigned N, typename U>
constexpr auto operator>(longint<T, N> const& a, U const b) noexcept
{
  return a > longint<T, N>(b);
}

template <typename T, unsigned N, typename U>
constexpr auto operator<=(longint<T, N> const& a, U const b) noexcept
{
  return a <= longint<T, N>(b);
}

template <typename T, unsigned N, typename U>
constexpr auto operator>=(longint<T, N> const& a, U const b) noexcept
{
  return a >= longint<T, N>(b);
}

#if __cplusplus > 201703L
template <typename T, unsigned N, typename U>
constexpr auto operator<=>(longint<T, N> const& a, U const b) noexcept
{
  return (a > b) - (a < b);
}
#endif

//
template <typename T, unsigned N>
std::string to_string(longint<T, N> a)
{
  std::string r;

  bool negative(a < 0);

  do
  {
    // auto const r(a % 10);
    // r.insert(0, 1, '0' + negative ? -r : r);
    // a /= 10;
  }
  while (a);

  if (negative)
  {
    r.insert(0, 1, '-');
  }

  return r;
}

template <typename T, unsigned N>
inline auto& operator<<(std::ostream& os, dpp::longint<T, N> const& p)
{
  return os << to_string(p);
}

}

namespace std
{

template <typename T, unsigned N>
struct hash<dpp::longint<T, N>>
{
  auto operator()(dpp::longint<T, N> const& a) const noexcept
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
