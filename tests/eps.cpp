//https://en.wikipedia.org/wiki/Machine_epsilon#Approximation
#include <iostream>
#include <iomanip>

#include "../dpp.hpp"

template <typename T, auto B = 2>
auto eps() noexcept
{
  T e(1);

  while ((1 + e / B) != 1) e /= B;

  return e;
}

int main()
{
  std::cout << std::fixed << std::setprecision(37);
  std::cout << eps<float>() << std::endl;
  std::cout << eps<double>() << std::endl;
  std::cout << eps<long double>() << std::endl;
  std::cout << eps<dpp::d16, 10>() << std::endl;
  std::cout << eps<dpp::d32, 10>() << std::endl;
  std::cout << eps<dpp::d64, 10>() << std::endl;
  std::cout << eps<dpp::d128, 10>() << std::endl;

  return 0;
}
