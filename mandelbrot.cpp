// https://github.com/dario-marvin/Mandelbrot
#include <cstdlib>

#include <iostream>

#include "dpp.hpp"

constexpr auto char_("o");

constexpr auto black("\033[22;30m");
constexpr auto red("\033[22;31m");
constexpr auto l_red("\033[01;31m");
constexpr auto green("\033[22;32m");
constexpr auto l_green("\033[01;32m");
constexpr auto orange("\033[22;33m");
constexpr auto yellow("\033[01;33m");
constexpr auto blue("\033[22;34m");
constexpr auto l_blue("\033[01;34m");
constexpr auto magenta("\033[22;35m");
constexpr auto l_magenta("\033[01;35m");
constexpr auto cyan("\033[22;36m");
constexpr auto l_cyan("\033[01;36m");
constexpr auto gray("\033[22;37m");
constexpr auto white("\033[01;37m");

//using D = float;
using D = dpp::d32;

auto mb(D const r, D const i) noexcept
{
  constexpr auto limit = 100;

  auto zr(r), zi(i);

  for (int j{}; j != limit; ++j)
  {
    if (auto const zr2(zr * zr), zi2(zi * zi); zr2 + zi2 > 4)
    {
      return j;
    }
    else
    {
      zr = zr2 - zi2 + r;
      zi = D(2) * zr * zi + i;
    }
  }

  return limit;
}

int main()
{
  auto const w(WEXITSTATUS(std::system("exit `tput cols`")));
  auto const h(WEXITSTATUS(std::system("exit `tput lines`")) - 1);

  D const x0(-2), y0(1);
  D const x1(1), y1(-1);

  auto const dx((x1 - x0) / D(w));
  auto const dy((y1 - y0) / D(h));

  auto y(y0 + dy / D(2));

  for (int i{}; i != h; ++i, y += dy)
  {
    auto x(x0 + dx / D(2));

    for (int j{}; j != w; ++j, x += dx)
    {
      if (auto const value(mb(x, y)); value == 100) {std::cout << " ";}
      else if (value > 90) {std::cout << red << char_;}
      else if (value > 70) {std::cout << l_red << char_;}
      else if (value > 50) {std::cout << orange << char_;}
      else if (value > 30) {std::cout << yellow << char_;}
      else if (value > 20) {std::cout << l_green << char_;}
      else if (value > 10) {std::cout << green << char_;}
      else if (value > 5) {std::cout << l_cyan << char_;}
      else if (value > 4) {std::cout << cyan << char_;}
      else if (value > 3) {std::cout << l_blue << char_;}
      else if (value > 2) {std::cout << blue << char_;}
      else if (value > 1) {std::cout << magenta << char_;}
      else {std::cout << l_magenta << char_;}
    }
  }

  std::cout << "\033[0m";

  return 0;
}
