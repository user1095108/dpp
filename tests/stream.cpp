#include <iostream>

#include "../dpp.hpp"

int main()
{
  using D = dpp::d32;

  for (;;)
  {
    D a;
    std::cin >> a;
    std::cout << a << " error: " << !std::cin << std::endl;
    std::cin.clear();
  }

  return 0;
}
