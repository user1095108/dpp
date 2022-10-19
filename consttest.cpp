#include <iostream>

#include "dpp.hpp"

using namespace dpp::literals;

template <auto a>
void test()
{
  std::cout << a << std::endl;
}

int main()
{
  test<dpp::d32{dpp::nan{}}>();
  test<1 / dpp::d32{3}>();
  test<.123_d32>();
  test<0._d32>();
  test<.0_d32>();
  test<0_d32 / 0_d32>();
  test<.1_d32 + .2_d32>();

  return 0;
}
