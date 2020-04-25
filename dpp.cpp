#include <iostream>

#include "dpp.hpp"

int main()
{
  dec32 nan(dec32::nan_{});
  std::cout << nan.is_nan() << " " << nan.exponent() << std::endl;

  dec32 a(123, -2);
  dec32 b(456, -1);

  auto const tmp(a * b);

  std::cout << tmp.mantissa() << std::endl;
  std::cout << tmp.exponent() << std::endl;

  return 0;
}
