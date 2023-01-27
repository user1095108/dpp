#include <iostream>

#include "sqrt.hpp"

int main()
{
  std::cout << dpp::sqrt(dpp::d256(2)) << std::endl;
  std::cout << dpp::sqrt(dpp::d256(3)) << std::endl;

  return 0;
}
