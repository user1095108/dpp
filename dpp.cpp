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
  T x, xn(S), e, en(S);

  do
  {
    x = xn;
    //xn = (x + S/x) / 2;
    xn = ((x * x + T(3) * S) / (T(3) * x * x + S)) * x;

    e = en;
    en = xn - x;
  }
  while (dpp::abs(en) < dpp::abs(e));

  return xn;
}

int main()
{
  //
  std::cout << "1000.0123"_d32 << std::endl;;
  std::cout << dpp::dec64(1000) + dpp::dec32(.0123f) << std::endl;;
  std::cout << dpp::dec64(-3.14) << std::endl;
  std::cout << dpp::to_float<float>(dpp::dec64(-3.14)) << std::endl;

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
  std::cout << std::sqrt(3.) << " " << sqrt("3"_d64) << std::endl;
  std::cout << std::sqrt(5.) << " " << sqrt("5"_d64) << std::endl;
  std::cout << std::sqrt(7.) << " " << sqrt("7"_d64) << std::endl;
  std::cout << std::sqrt(9.f) << " " << sqrt("9"_d32) << std::endl;
  std::cout << std::sqrt(77.) << " " << sqrt("77"_d64) << std::endl;

  //
  std::cout << std::endl;
  auto a(dpp::to_decimal<dpp::dec64>("1.23"));
  auto b(dpp::to_decimal<dpp::dec64>("45.6"));

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
