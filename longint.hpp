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

template <typename T, std::size_t N>
class longint
{

static_assert(std::is_unsigned_v<T>);
static_assert(N > 0);

std::array<T, N> v_;

public:
enum : T { max_e = std::numeric_limits<T>::max() };

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
longint(U v) noexcept
{
  std::size_t i{};

  do
  {
    v_[i++] = v & max_e;
    v /= (max_e + 1);
  }
  while (v);
}

constexpr explicit operator bool() const noexcept
{
  return any(*this);
}

constexpr auto& operator[](std::size_t const i) noexcept
{
  return v_[i];
}

constexpr auto& operator[](std::size_t const i) const noexcept
{
  return v_[i];
}

//
constexpr auto operator~() const noexcept
{
  auto const negate([&]<std::size_t ...I>(
    std::index_sequence<I...>) noexcept
    {
      return decltype(v_){T(~v_[I])...};
    }
  );

  return longint(negate(std::make_index_sequence<N>()));
}

//
constexpr auto operator-() const noexcept
{
  return ~(*this) + longint(1);
}

template <typename U, std::size_t O, std::size_t P>
friend constexpr bool operator==(longint<U, O> const&,
  longint<U, P> const&) noexcept;

};

//misc////////////////////////////////////////////////////////////////////////
template <typename T, std::size_t N>
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

//arithmetic//////////////////////////////////////////////////////////////////
template <typename T, std::size_t M, std::size_t N>
constexpr auto operator+(longint<T, M> const& a,
  longint<T, N> const& b) noexcept
{
  longint<T, std::max(M, N)> r(M > N ? a : b);

  auto const add([&]<std::size_t ...I>(
    std::index_sequence<I...>) noexcept
    {
      bool carry{};

      T tmp;

      (
        (
          tmp = (I < M ? a[I] : T{}) + (I < N ? b[I] : T{}) + carry,
          r[I] = tmp,
          carry = (tmp < a[I]) || (tmp < b[I]) || (tmp < carry)
        ),
        ...
      );
    }
  );

  add(std::make_index_sequence<std::max(M, N)>());

  return r;
}

template <typename T, std::size_t M, std::size_t N>
constexpr auto operator-(longint<T, M> const& a,
  longint<T, N> const& b) noexcept
{
  return a + (-b);
}

//comparison//////////////////////////////////////////////////////////////////
template <typename T, std::size_t M, std::size_t N>
constexpr bool operator==(longint<T, M> const& a,
  longint<T, N> const& b) noexcept
{
  return a.v_ == b.v_;
}

template <typename T, std::size_t M, std::size_t N>
constexpr bool operator!=(longint<T, M> const& a,
  longint<T, N> const& b) noexcept
{
  return !(a == b);
}

//
template <typename T, std::size_t M, std::size_t N>
constexpr bool operator<(longint<T, M> const& a,
  longint<T, N> const& b) noexcept
{
  auto const r(a - b);

  return r[std::max(M, N) - 1] & ((decltype(r)::max_e >> 1) + 1);
}

template <typename T, std::size_t M, std::size_t N>
constexpr auto operator>(longint<T, M> const& a,
  longint<T, N> const& b) noexcept
{
  return b < a;
}

template <typename T, std::size_t M, std::size_t N>
constexpr auto operator<=(longint<T, M> const& a,
  longint<T, N> const& b) noexcept
{
  return !(b < a);
}

template <typename T, std::size_t M, std::size_t N>
constexpr auto operator>=(longint<T, M> const& a,
  longint<T, N> const& b) noexcept
{
  return !(a < b);
}

#if __cplusplus > 201703L
template <typename T, std::size_t M, std::size_t N>
constexpr auto operator<=>(longint<T, M> const& a,
  longint<T, N> const& b) noexcept
{
  return (a > b) - (a < b);
}
#endif

//////////////////////////////////////////////////////////////////////////////
template <typename T, std::size_t N>
std::string to_string(longint<T, N> a)
{
  std::string r;

  do
  {
  }
  while (a);

  return r;
}

}

template <typename T, std::size_t N>
inline auto& operator<<(std::ostream& os, dpp::longint<T, N> const& p)
{
  return os << to_string(p);
}

namespace std
{

template <typename T, std::size_t N>
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
