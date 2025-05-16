#ifndef DPP_UTILS_HPP
# define DPP_UTILS_HPP
# pragma once

#include "dpp.hpp"

namespace dpp
{

//
template <auto B, typename T, typename E>
constexpr dpp<T, E> div(dpp<T, E> const& a) noexcept
{
  return a * ar::coeff<inv(dpp<T, E>(B))>();
}

template <typename T, typename E>
constexpr dpp<T, E> div2(dpp<T, E> const& a) noexcept
{
  using U = typename dpp<T, E>::sig2_t;
  using F = typename dpp<T, E>::exp2_t;

  if (isnan(a)) [[unlikely]] return nan;

  return dpp<T, E>(U(5) * U(a.m_), F(a.e_) - F(1));
}

template <typename T, typename E>
constexpr dpp<T, E> div4(dpp<T, E> const& a) noexcept
{
  using U = typename dpp<T, E>::sig2_t;
  using F = typename dpp<T, E>::exp2_t;

  if (isnan(a)) [[unlikely]] return nan;

  return dpp<T, E>(U(25) * U(a.m_), F(a.e_) - F(2));
}

template <typename T, typename E>
constexpr dpp<T, E> div5(dpp<T, E> const& a) noexcept
{
  using U = typename dpp<T, E>::sig2_t;
  using F = typename dpp<T, E>::exp2_t;

  if (isnan(a)) [[unlikely]] return nan;

  return dpp<T, E>(U(a.m_) << 1, F(a.e_) - F(1));
}

template <typename T, typename E>
constexpr dpp<T, E> div8(dpp<T, E> const& a) noexcept
{
  using U = typename dpp<T, E>::sig2_t;
  using F = typename dpp<T, E>::exp2_t;

  if (isnan(a)) [[unlikely]] return nan;

  return dpp<T, E>(U(125) * U(a.m_), F(a.e_) - F(3));
}

template <auto E10 = 1, typename T, typename E>
constexpr dpp<T, E> div10(dpp<T, E> const& a) noexcept
{
  using F = typename dpp<T, E>::exp2_t;

  if (isnan(a)) [[unlikely]] return nan;

  return dpp<T, E>(a.m_, F(a.e_) - F(E10));
}

template <auto E2 = 1, typename T, typename E>
constexpr dpp<T, E> mul2(dpp<T, E> const& a) noexcept
{
  using U = typename dpp<T, E>::sig2_t;
  using F = typename dpp<T, E>::exp2_t;

  if (isnan(a)) [[unlikely]] return nan;

  return dpp<T, E>(U(a.m_) << E2, F(a.e_));
}

}

#endif // DPP_UTILS_HPP
