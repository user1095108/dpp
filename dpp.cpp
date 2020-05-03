#include <iostream>

#include "dpp.hpp"

using namespace dpp::literals;

template <typename T>
constexpr auto sqrt(T const S) noexcept
{
  T x;
  T xn(S);

  T e;
  T en(S);

  do
  {
    x = xn;
    xn = (x + S/x) / T(2);

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
  std::cout << dpp::dec32(1000.0123f) << std::endl;;
  std::cout << dpp::dec64(-3.14) << std::endl;;

  //
  std::cout << sqrt("2"_d32) << std::endl;
  std::cout << sqrt("3"_d64) << std::endl;
  std::cout << sqrt("9"_d32) << std::endl;

  //
  auto const a(dpp::to_decimal<dpp::dec64>("1.23"));
  auto const b(dpp::to_decimal<dpp::dec64>("45.6"));

  std::cout << a + b << std::endl;
  std::cout << a - b << std::endl;
  std::cout << a * b << std::endl;
  std::cout << a / b << std::endl;
  std::cout << b / a << std::endl;

  //
  std::cout << dpp::ceil(a) << std::endl;
  std::cout << dpp::floor(b) << std::endl;
  std::cout << dpp::round(b) << std::endl;

  return 0;
}
