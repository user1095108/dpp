#include <iostream>

#include "sqrt.hpp"

using namespace dpp::literals;

int main()
{
  std::cout << dpp::sqrt(2_d16) << std::endl;
  std::cout << dpp::sqrt(2_d24) << std::endl;
  std::cout << dpp::sqrt(2_d32) << std::endl;
  std::cout << dpp::sqrt(2_d48) << std::endl;
  std::cout << dpp::sqrt(2_d64) << std::endl;
  std::cout << dpp::sqrt(2_d96) << std::endl;
  std::cout << dpp::sqrt(2_d256) << std::endl;

  //
  std::cout << 1 / (1 + dpp::sqrt(2_d256)) << std::endl;
  std::cout << 2 / (1 + dpp::sqrt(5_d256)) << std::endl;

  //
  return 0;
}
