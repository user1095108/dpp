#include <iostream>

#include "../dpp.hpp"

int main()
{
  using D = dpp::d32;

  D a;
  std::cin >> a;
  std::cout << a << " error: " << !std::cin << std::endl;

  return 0;
}
