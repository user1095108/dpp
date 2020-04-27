#include <iostream>

#include "dpp.hpp"

template <typename T>
constexpr auto sqrt(T const S) noexcept
{
  constexpr auto half(dpp::to_decimal<T>(".5"));

  T x;
  T xn(S);

  do
  {
    x = xn;
    xn = half * (xn + S/xn);
  }
  while (x != xn);

  return xn;
}

int main()
{
  std::cout << sqrt(dpp::dec32(2)) << std::endl;
  std::cout << sqrt(dpp::dec32(3)) << std::endl;
  std::cout << sqrt(dpp::dec32(9)) << std::endl;

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
