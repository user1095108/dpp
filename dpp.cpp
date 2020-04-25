#include <iostream>

#include "dpp.hpp"

int main()
{
  std::cout << dec16::nan.is_nan() << std::endl;

  dec16 a(12, -2);
  dec16 b(45, -1);

  auto const tmp(a * b);

  std::cout << tmp.mantissa() << std::endl;
  std::cout << tmp.exponent() << std::endl;

  return 0;
}
