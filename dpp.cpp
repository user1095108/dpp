#if !defined(__ARM_ARCH) && !defined(__clang__)
#include <decimal/decimal>
#endif

#include <iostream>

#include <iomanip>

#include "dpp.hpp"

using namespace dpp::literals;

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
//  xn = (xo + S/xo) / 2;

    en = xo - xn;
  }
  while (abs(en) < abs(eo));

  return (xo + xn) / T(2);
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
    euler("1"_d64, "0"_d64, "1"_d64, ".000001"_d64, f) << std::endl;
}

void comp_sqrt32(unsigned const s) noexcept
{
  std::cout << std::sqrt(float(s)) << " " <<
    ssqrt(float(s)) << " " <<
#if !defined(__ARM_ARCH) && !defined(__clang__)
    to_string(ssqrt(std::decimal::decimal32(s))) << " " <<
#endif
    ssqrt(dpp::d32(s)) << std::endl;
}

void comp_sqrt64(unsigned const s) noexcept
{
  std::cout << std::sqrt(double(s)) << " " <<
    ssqrt(double(s)) << " " <<
#if !defined(__ARM_ARCH) && !defined(__clang__)
    to_string(ssqrt(std::decimal::decimal64(s))) << " " <<
#endif
    ssqrt(dpp::d64(s)) << std::endl;
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

  std::cout << trapezoidal(1., 5., 1000, f3) << " " <<
#if !defined(__ARM_ARCH) && !defined(__clang__)
    to_string(trapezoidal(std::decimal::decimal64(1),
      std::decimal::decimal64(5), 1000, f3)) << " " <<
#endif
    trapezoidal(dpp::d64(1), dpp::d64(5), 1000, f3) << std::endl;
}

int main()
{
  std::cout << std::setprecision(17);

  //
  std::cout << ("3.1622775"_d32 + "3.1622778"_d32) / dpp::d32(2) << std::endl;

  std::cout << -"1000.0123"_d32 << std::endl;
  std::cout << dpp::d32(.0123f) + dpp::d64(1000) << std::endl;
  std::cout << dpp::d64(-M_PI) << std::endl;
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
  auto a(dpp::to_decimal<dpp::d64>("1.23"));
  auto b(dpp::to_decimal<dpp::d64>("45.6"));

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

  return 0;
}
