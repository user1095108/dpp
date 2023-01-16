#if !defined(__ARM_ARCH) && !defined(__clang__)
#include <decimal/decimal>
#endif

#include <iostream>

#include <iomanip>

#include "dpp.hpp"

using namespace dpp::literals;

#ifndef M_PI
# define M_PI 3.14159265358979323846264338327950288
#endif // M_PI

template <typename T>
constexpr auto abs(T const n) noexcept
{
  return n < 0 ? -n : n;
}

#if !defined(__ARM_ARCH) && !defined(__clang__)
template <typename D>
std::string to_string(D d)
{
  std::string r;

  if (d < D{})
  {
    d = abs(d);
    r.append(1, '-');
  }

  int e{};

  while (d >= D{1})
  {
    d /= 10;
    ++e;
  }

  if (!e || (d == D{}))
  {
    r.append(1, '0');
  }

  while (d != D{})
  {
    if (!e--)
    {
      r.append(1, '.');
    }

    long long di(d *= 10);
    d -= di;

    r.append(1, di + '0');
  }

  return r;
}
#endif

template <typename T, typename F>
constexpr auto euler(T y, T t,  T const t1, T const h, F const f) noexcept
{
  while (t < t1)
  {
    y += h * f(y, t);
    t += h;
  }

  return y;
}

template <typename T, typename F>
constexpr auto trapezoidal(T t,  T const t1, unsigned const N, F const f) noexcept
{
  T const dt((t1 - t) / T(N));

  T y(f(t) + f(t1));
  t += dt;

  T s{};

  while (t < t1)
  {
    s += f(t);
    t += dt;
  }

  return dt * (y / T(2) + s);
}

template <typename T>
constexpr auto ssqrt(T const S) noexcept
{
  T xo, xn(S), eo, en(S);

  do
  {
    xo = xn;
    eo = en;

    auto const xs(xo * xo);
    xn = ((xs + T(3) * S) / (T(3) * xs + S)) * xo;
//  xn = (xo + S/xo) / T(2);

    en = xo - xn;
  }
  while (abs(en) < abs(eo));

  return (xo + xn) / T(2);
}

template <unsigned M>
inline auto exact_sqrt(dpp::dpp<M> const& a) noexcept
{
  using T = typename dpp::dpp<M>::mantissa_type;
  using U = typename std::make_unsigned<T>::type;
  using V = intt::intt<U, 3>;

  V m(intt::direct{}, U(a.mantissa()));
  dpp::int_t e(a.exponent());

  for (; m <= V::max() / 10; m *= 10, --e);
  if (e % 2) { ++e; m /= 10; }

  return dpp::dpp<M>(intt::seqsqrt(m), e / 2);
}

void comp_euler64() noexcept
{
  auto const f([](auto const& y, auto const&) noexcept
    {
      return y;
    }
  );

  std::cout << 
    euler(1., 0., 1., .000001, f) << " " <<
#if !defined(__ARM_ARCH) && !defined(__clang__)
    to_string(euler(std::decimal::decimal64(1),
      std::decimal::decimal64(0),
      std::decimal::decimal64(1),
      std::decimal::decimal64(.000001), f)) << " " <<
#endif
    euler(1_d64, 0_d64, 1_d64, .000001_d64, f) << std::endl;
}

void comp_sqrt32(unsigned const s) noexcept
{
  std::cout << std::sqrt(float(s)) << " " <<
    ssqrt(float(s)) << " " <<
#if !defined(__ARM_ARCH) && !defined(__clang__)
    to_string(ssqrt(std::decimal::decimal32(s))) << " " <<
#endif
    ssqrt(dpp::d32(s)) << " " <<
    exact_sqrt(dpp::d32(s)) << std::endl;
}

void comp_sqrt64(unsigned const s) noexcept
{
  std::cout << std::sqrt(double(s)) << " " <<
    ssqrt(double(s)) << " " <<
#if !defined(__ARM_ARCH) && !defined(__clang__)
    to_string(ssqrt(std::decimal::decimal64(s))) << " " <<
#endif
    ssqrt(dpp::d64(s)) << " " <<
    exact_sqrt(dpp::d64(s)) << std::endl;
}

void comp_trapezoidal64() noexcept
{
  auto const f1([](auto const t) noexcept { return t * t; });

  std::cout << trapezoidal(0., 1., 1000, f1) << " " <<
#if !defined(__ARM_ARCH) && !defined(__clang__)
    to_string(trapezoidal(std::decimal::decimal64(0),
      std::decimal::decimal64(1), 1000, f1)) << " " <<
#endif
    trapezoidal(dpp::d64(0), dpp::d64(1), 1000, f1) << std::endl;

  auto const f2([](auto const t) noexcept { return t*t*t; });

  std::cout << trapezoidal(-1., 1., 1000, f2) << " " <<
#if !defined(__ARM_ARCH) && !defined(__clang__)
    to_string(trapezoidal(std::decimal::decimal64(-1),
      std::decimal::decimal64(1), 1000, f2)) << " " <<
#endif
    trapezoidal(dpp::d64(-1), dpp::d64(1), 1000, f2) << std::endl;

  auto const f3([](auto const t) noexcept { return decltype(t)(1) / t; });

  std::cout << trapezoidal(1., 5., 10000, f3) << " " <<
#if !defined(__ARM_ARCH) && !defined(__clang__)
    to_string(trapezoidal(std::decimal::decimal64(1),
      std::decimal::decimal64(5), 10000, f3)) << " " <<
#endif
    trapezoidal(dpp::d64(1), dpp::d64(5), 10000, f3) << std::endl;
}

int main()
{
  std::cout << std::setprecision(17);

  //
  std::cout << -.001_d32 << " " <<
    std::hash<dpp::d32>()(-.001_d32) << std::endl;
  std::cout << -2_d32 / -3_d64 << " " << -2_d32 / 3_d64 << " " <<
    2_d64 / -3_d64 << " " << 2_d32 / 3_d32 << std::endl;
  std::cout << (3.1622775_d32 + 3.1622778_d32) / dpp::d32(2) << std::endl;

  std::cout << -1000.0123_d32 << std::endl;
  std::cout << dpp::d32(.0123f) + dpp::d64(1000) << std::endl;
  std::cout << dpp::d64((long double)(-M_PI)) << " " << dpp::d32(1.23456e20f) << std::endl;
  std::cout << dpp::d32(M_PI) << " " << -float(dpp::d32(-M_PI)) << std::endl;

  //
  std::cout << std::endl;
  comp_euler64();

  //
  std::cout << std::endl;
  comp_sqrt32(2);
  comp_sqrt64(2);
  comp_sqrt64(3);
  comp_sqrt32(5);
  comp_sqrt64(5);
  comp_sqrt64(7);
  comp_sqrt32(9);
  comp_sqrt64(9);
  comp_sqrt64(10);
  comp_sqrt64(77);

  //
  std::cout << std::endl;
  comp_trapezoidal64();

  //
  std::cout << std::endl;

  std::cout << .1_d16 + .2_d64 << std::endl;

  auto const a(dpp::to_decimal<dpp::d64>("1.23"));
  auto const b(dpp::to_decimal<dpp::d64>("45.6"));

  std::cout << a << std::endl;
  std::cout << b << std::endl;

  std::cout << a + b << std::endl;
  std::cout << a - b << std::endl;
  std::cout << a * b << std::endl;
  std::cout << a / b << std::endl;
  std::cout << b / a << std::endl;

  //
  std::cout << dpp::ceil(b) << std::endl;
  std::cout << dpp::floor(b) << std::endl;
  std::cout << dpp::round(b) << std::endl;

  //
  std::cout << (dpp::d32(dpp::nan{}) != dpp::d64(dpp::nan{})) << std::endl;

  //
  return a <=> b > 0;
}
