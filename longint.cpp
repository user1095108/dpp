#include <cstdint>

#include <iostream>

#include "longint.hpp"

int main()
{
//using D = longint::longint<std::uint8_t, 8>;
//using D = longint::longint<std::uint32_t, 4>;
  using D = longint::longint<std::uint64_t, 2>;

  //
  D a(-1025);
  D b(-1024);

  //
  {
    std::cout << int(D(-2) >> 1) << std::endl;
    std::cout << int(D(1) << 11) << std::endl;
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
