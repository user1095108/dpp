#include <iostream>

#include "dpp.hpp"

int main()
{
  dpp::dec32 const nan(dpp::dec32::nan_{});
  std::cout << nan.is_nan() << std::endl;

  auto const a(dpp::to_decimal<dpp::dec32>("1.23"));
  auto const b(dpp::to_decimal<dpp::dec32>("45.6"));

  std::cout << a + b << std::endl;
  std::cout << a - b << std::endl;
  std::cout << a * b << std::endl;
  std::cout << a / b << std::endl;
  std::cout << b / a << std::endl;

  return 0;
}
