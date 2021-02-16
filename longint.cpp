#include <cstdint>

#include <iostream>

#include "longint.hpp"

int main()
{
  using D = longint::longint<std::uint16_t, 8>;
//using D = longint::longint<std::uint32_t, 4>;
//using D = longint::longint<std::uint64_t, 2>;

  //
  D a(-1025);
  D b(-1024);

  //
  {
    std::cout << to_raw(D(-11)) << std::endl;
    std::cout << to_raw(D(-10) + D(-1)) << std::endl;
    std::cout << int(D(1) << 11) << " : " << to_raw(D(1) << 11) << std::endl;
    std::cout << int(D(-2) >> 1) << " : " << to_raw(D(-2) >> 1) << std::endl;
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
    std::cout << int(D(1049600)) << " : " << to_raw(D(1049600)) << std::endl;
    std::cout << to_raw(a) << std::endl;
    std::cout << to_raw(b) << std::endl;
    std::cout << int(a * b) << " : " << to_raw(a * b) << std::endl;
    //std::cout << (a * b) << std::endl;
  }

  //
  return 0;
}
