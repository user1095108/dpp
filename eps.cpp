//https://en.wikipedia.org/wiki/Machine_epsilon#Approximation
#include <iostream>

#include "dpp.hpp"

template <typename T>
auto eps() noexcept
{
  T e(1);

  while ((1 + e / 2) != 1)
  {
    e /= 2;
  }

  return e;
}

int main()
{
  std::cout << eps<float>() << std::endl;
  std::cout << eps<double>() << std::endl;
  std::cout << eps<long double>() << std::endl;
  std::cout << eps<dpp::d16>() << std::endl;
  std::cout << eps<dpp::d32>() << std::endl;
  std::cout << eps<dpp::d64>() << std::endl;

  return 0;
}
