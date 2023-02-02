#include <iostream>

#include "sqrt.hpp"

int main()
{
  std::cout << dpp::sqrt(dpp::d16(2)) << std::endl;
  std::cout << dpp::sqrt(dpp::d24(2)) << std::endl;
  std::cout << dpp::sqrt(dpp::d32(2)) << std::endl;
  std::cout << dpp::sqrt(dpp::d48(2)) << std::endl;
  std::cout << dpp::sqrt(dpp::d64(2)) << std::endl;
  std::cout << dpp::sqrt(dpp::d96(2)) << std::endl;
  std::cout << dpp::sqrt(dpp::d256(2)) << std::endl;

  //
  auto const igr(2 / (1 + dpp::sqrt(dpp::d256(5))));

  std::cout << igr << std::endl;

  std::cout << std::hex << std::uint64_t(igr * (std::uint64_t(1) << 32)) << std::endl;
  std::cout << std::uint64_t(igr * dpp::detail::pow<dpp::d256, 2>(64)) << std::endl;
  std::cout << intt::magic::IGR << std::endl;
  std::cout << intt::intt<std::uint64_t, 3>(igr * dpp::detail::pow<dpp::d256, 2>(128)) << std::endl;

  return 0;
}
