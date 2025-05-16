#include <iostream>

#include "../utils.hpp"

using namespace dpp::literals;

int main()
{
  std::cout << dpp::div<3>(1_d32) << std::endl;
  std::cout << dpp::div2(1_d32) << std::endl;
  std::cout << dpp::mul2(1_d32) << std::endl;
  std::cout << dpp::div10<1>(1_d32) << std::endl;

  return 0;
}
