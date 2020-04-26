#include <iostream>

#include "dpp.hpp"

int main()
{
  dpp::dec64 const nan(dpp::dec64::nan{});
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
