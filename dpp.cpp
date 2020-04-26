#include <decimal/decimal>

#include <iostream>

#include "dpp.hpp"

template <typename T>
constexpr auto sqrt1(T const S) noexcept
{
  constexpr auto half(dpp::to_decimal<T>(".5"));

  T xn(S);

  xn = half * (xn + S/xn);
  xn = half * (xn + S/xn);
  xn = half * (xn + S/xn);
  xn = half * (xn + S/xn);

  return xn;
}

template <typename T>
constexpr auto sqrt2(T const n) noexcept
{
  T x;
  T xn(n);

  do
  {
    x = xn;
    xn = (x + n/x) / 2;
  }
  while (x != xn);

  return x;
}

int main()
{
  std::cout << sqrt1(dpp::dec32(2)) << std::endl;
  std::cout << sqrt2(dpp::dec32(2)) << std::endl;
  std::cout << sqrt1(dpp::dec32(3)) << std::endl;
  std::cout << sqrt2(dpp::dec32(3)) << std::endl;

  std::cout << sqrt1(dpp::dec32(9)) << std::endl;
  std::cout << sqrt2(dpp::dec32(9)) << std::endl;

  //
  auto const a(dpp::to_decimal<dpp::dec32>("1.23"));
  auto const b(dpp::to_decimal<dpp::dec32>("45.6"));

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
