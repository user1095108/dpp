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
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }

  return 0;
}
