#include <iostream>

#include "../sqrt.hpp"

using namespace dpp::literals;

int main()
{
  std::cout << dpp::sqrt(2_d16) << std::endl;
  std::cout << dpp::sqrt(2_d24) << std::endl;
  std::cout << dpp::sqrt(2_d32) << std::endl;
  std::cout << dpp::sqrt(2_d48) << std::endl;
  std::cout << dpp::sqrt(dpp::to_decimal<dpp::dpp<intt::intt<std::uint8_t, 7>, intt::intt<std::uint8_t, 2>>>("2")) << std::endl;
  std::cout << dpp::sqrt(2_d64) << std::endl;
  std::cout << dpp::sqrt(2_d96) << std::endl;
  std::cout << dpp::sqrt(2_d128) << std::endl;
  std::cout << dpp::sqrt(2_d256) << std::endl;
  std::cout << dpp::sqrt(2_d512) << std::endl;

  //
  std::cout << 1 / (1 + dpp::sqrt(2_d512)) << std::endl;
  std::cout << 2 / (1 + dpp::sqrt(5_d512)) << std::endl;

  //
  return 0;
}
