#include <cstdint>

#include <iostream>

#include "longint.hpp"

int main()
{
  dpp::longint<std::uint8_t, 2> a(255);
  dpp::longint<std::uint8_t, 2> b(1);

  auto r(a + b);

  std::cout << unsigned((a + b)[0]) << std::endl;
  std::cout << unsigned((a - b)[0]) << std::endl;
  std::cout << (a == b) << std::endl;
  std::cout << ((a + b) > (a - b)) << std::endl;

  return 0;
}
