#include <cstdint>

#include <iostream>

#include "longint.hpp"

int main()
{
  using D = longint::longint<std::uint8_t, 8>;

  //
  D a(1024);
  D b(-10);

  //
  {
    std::cout << int(D(1) << 11) << std::endl;
    std::cout << int(D(-1)) << std::endl;
  }

  //
  {
    std::cout << "== " << (a == b) << std::endl;
    std::cout << "> " << (a > b) << std::endl;
    std::cout << "< " << (a < b) << std::endl;
    std::cout << int(a + b) << std::endl;
    std::cout << int(a - b) << std::endl;
  }

  //
  {
    std::cout << int(a * b) << std::endl;
  }

  //
  return 0;
}
