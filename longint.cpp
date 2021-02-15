#include <cstdint>

#include <iostream>

#include "longint.hpp"

int main()
{
  using D = dpp::longint<std::uint8_t, 8>;

  //
  D a(1024);
  D b(-1025);

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
