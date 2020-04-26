#include <iostream>

#include "dpp.hpp"

template <typename T>
inline auto sqrt(T const S) noexcept
{
  auto xn(S);

  auto const half(dpp::to_decimal<dpp::dec32>(".5"));

  xn = half * (xn + S/xn);
  xn = half * (xn + S/xn);
  xn = half * (xn + S/xn);

  return xn;
}

int main()
{
  std::cout << sqrt(dpp::dec32(3)) << std::endl;
  std::cout << sqrt(dpp::dec32(2)) << std::endl;

  auto const a(dpp::to_decimal<dpp::dec32>("1.23"));
  auto const b(dpp::to_decimal<dpp::dec32>("45.6"));

  std::cout << a + b << std::endl;
  std::cout << a - b << std::endl;
  std::cout << a * b << std::endl;
  std::cout << a / b << std::endl;
  std::cout << b / a << std::endl;

  return 0;
}
