#include <iostream>

#include "dpp.hpp"

int main()
{
  dpp::dec32 nan(dpp::dec32::nan_{});
  std::cout << nan.is_nan() << std::endl;

  dpp::dec32 a(123, -2);
  dpp::dec32 b(456, -1);

  auto const tmp(a + b);

  std::cout << tmp.mantissa() << std::endl;
  std::cout << tmp.exponent() << std::endl;

  return 0;
}
