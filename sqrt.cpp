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
  auto const igr(2 / (1 + dpp::sqrt(5_d256)));

  std::cout << igr << std::endl;

  std::cout << std::hex << std::uint64_t(igr * (std::uint64_t(1) << 32)) << std::endl;
  std::cout << std::uint64_t(igr * dpp::detail::pow<dpp::d256, 2>(64)) << std::endl;
  std::cout << intt::magic::IGR << std::endl;
  std::cout << intt::intt<std::uint64_t, 3>(igr * dpp::detail::pow<dpp::d256, 2>(128)) << std::endl;

  return 0;
}
