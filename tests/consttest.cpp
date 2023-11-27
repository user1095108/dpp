#include <iostream>

#include "../dpp.hpp"

using namespace dpp::literals;

template <auto a>
void test()
{
  std::cout << a << std::endl;
}

int main()
{
  test<dpp::d32{dpp::nan}>();
  test<dpp::inv(3_d32)>();
  test<dpp::inv(6_d32)>();
  test<dpp::inv(11_d32)>();
  test<20_d64 / 3>();
  test<.123_d32>();
  test<0._d32>();
  test<.0_d64>();
  test<0_d32 / 0_d16>();
  test<.1_d32 + .2_d16>();
  test<dpp::d32::eps>();
  test<dpp::d64::eps>();

  return 0;
}
