// https://github.com/dario-marvin/Mandelbrot
#include <cstdlib>

#include <iostream>

#include "dpp.hpp"

constexpr char char_[] = "o";

constexpr char black[] = "\033[22;30m";
constexpr char red[] = "\033[22;31m";
constexpr char l_red[] = "\033[01;31m";
constexpr char green[] = "\033[22;32m";
constexpr char l_green[] = "\033[01;32m";
constexpr char orange[] = "\033[22;33m";
constexpr char yellow[] = "\033[01;33m";
constexpr char blue[] = "\033[22;34m";
constexpr char l_blue[] = "\033[01;34m";
constexpr char magenta[] = "\033[22;35m";
constexpr char l_magenta[] = "\033[01;35m";
constexpr char cyan[] = "\033[22;36m";
constexpr char l_cyan[] = "\033[01;36m";
constexpr char gray[] = "\033[22;37m";
constexpr char white[] = "\033[01;37m";

//using D = float;
using D = dpp::d32;

int mndlbrot(D const r, D const i) noexcept
{
  constexpr int limit = 100;

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

int main ()
{
  auto const w(WEXITSTATUS(std::system("exit `tput cols`")));
  auto const h(WEXITSTATUS(std::system("exit `tput lines`")) - 1);

  D x_start(-2), y_start(-1);
  D x_fin(1), y_fin(1);

  auto const dx((x_fin - x_start)/D(w - 1));
  auto const dy((y_fin - y_start)/D(h - 1));

  D y(y_fin);

  for (int i{}; i != h; ++i)
  {
    D x(x_start);

    for (int j{}; j != w; ++j)
    {
      if (auto const value(mndlbrot(x, y)); value == 100) {std::cout << " ";}
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

      std::cout << "\033[0m";

      x += dx;
    }

    y -= dy;
  }

  return 0;
}
