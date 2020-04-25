#include <iostream>

#include "dpp.hpp"

int main()
{
  dec64 a(123, -2);
  dec64 b(456, -1);

  auto const tmp(a * b);

  std::cout << tmp.mantissa() << std::endl;
  std::cout << tmp.exponent() << std::endl;

  return 0;
}
