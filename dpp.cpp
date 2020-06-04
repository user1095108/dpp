#include <iostream>

#include <iomanip>

#include "dpp.hpp"

using namespace dpp::literals;

template <typename T, typename F>
constexpr auto euler(T y, T t,  T const& t1, T const& h, F const f) noexcept
{
  while (t < t1)
  {
    y += h * f(y, t);

    t += h;
  }

  return y;
}

template <typename T>
constexpr auto sqrt(T const S) noexcept
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
  while (dpp::abs(en) < dpp::abs(eo));

/*
  xn = xo;
  en = S - xo * xo;

  if (dpp::sign(en))
  {
    do
    {
      xo = xn;
      eo = en;

      xn = T(xo.mantissa() - 1, xo.exponent());
      en = S - xn * xn;
    }
    while (dpp::abs(en) <= dpp::abs(eo));

    xn = xo;
    en = eo;

    do
    {
      xo = xn;
      eo = en;

      xn = T(xo.mantissa() + 1, xo.exponent());
      en = S - xn * xn;
    }
    while (dpp::abs(en) < dpp::abs(eo));
  }
*/

  return xn;
}

int main()
{
  //
  std::cout << ("3.1622775"_d32 + "3.1622778"_d32) / 2 << std::endl;

  std::cout << -"1000.0123"_d32 << std::endl;;
  std::cout << dpp::d32(.0123f) + dpp::d64(1000) << std::endl;;
  std::cout << dpp::d64(-M_PI) << std::endl;
  std::cout << dpp::d32(M_PI) << " " << -float(dpp::d32(-M_PI)) << std::endl;

  //
  std::cout << std::endl;
  std::cout << std::setprecision(17) <<
    euler(1., 0., 1., .000001,
      [](auto const& y, auto const&) noexcept
      {
        return y;
      }
    ) << " " <<
    euler("1"_d64, "0"_d64, "1"_d64, ".000001"_d64,
      [](auto const& y, auto const&) noexcept
      {
        return y;
      }
    ) << std::endl;

  //
  std::cout << std::endl;
  std::cout << std::sqrt(2.f) << " " << sqrt("2"_d32) << std::endl;
  std::cout << std::sqrt(2.) << " " << sqrt("2"_d64) << std::endl;
  std::cout << std::sqrt(3.f) << " " << sqrt("3"_d32) << std::endl;
  std::cout << std::sqrt(3.) << " " << sqrt("3"_d64) << std::endl;
  std::cout << std::sqrt(5.) << " " << sqrt("5"_d64) << std::endl;
  std::cout << std::sqrt(7.f) << " " << sqrt("7"_d32) << std::endl;
  std::cout << std::sqrt(7.) << " " << sqrt("7"_d64) << std::endl;
  std::cout << std::sqrt(9.f) << " " << sqrt("9"_d32) << std::endl;
  std::cout << std::sqrt(10.f) << " " << sqrt("10"_d32) << std::endl;
  std::cout << std::sqrt(10.) << " " << sqrt("10"_d64) << std::endl;
  std::cout << std::sqrt(77.f) << " " << sqrt("77"_d32) << std::endl;
  std::cout << std::sqrt(77.) << " " << sqrt("77"_d64) << std::endl;

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
