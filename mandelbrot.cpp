// https://github.com/dario-marvin/Mandelbrot
#include <cstdlib>

#include <iostream>

#include "dpp.hpp"

//using D = float;
using D = dpp::d32;

int mandelbrot(D const r, D const i) noexcept
{
  constexpr int limit = 100;

  D zr(r), zi(i);

  for (int j{}; j != limit; ++j)
  {
    D const zr2(zr * zr);
    D const zi2(zi * zi);

    if (zr2 + zi2 > 4.0) return j;

    zr = zr2 - zi2 + r;
    zi = D(2) * zr * zi + i;
  }

  return limit;
}

int main ()
{
  auto const w(WEXITSTATUS(std::system("exit `tput cols`")));
  auto const h(WEXITSTATUS(std::system("exit `tput lines`")) - 1);

  D x_start("-2.0");
  D x_fin("1.0");
  D y_start("-1.0");
  D y_fin("1.0");

  D dx((x_fin - x_start)/D(w - 1));
  D dy((y_fin - y_start)/D(h - 1));

  std::string char_ = "o";

  std::string black = "\033[22;30m";
  std::string red = "\033[22;31m";
  std::string l_red = "\033[01;31m";
  std::string green = "\033[22;32m";
  std::string l_green = "\033[01;32m";
  std::string orange = "\033[22;33m";
  std::string yellow = "\033[01;33m";
  std::string blue = "\033[22;34m";
  std::string l_blue = "\033[01;34m";
  std::string magenta = "\033[22;35m";
  std::string l_magenta = "\033[01;35m";
  std::string cyan = "\033[22;36m";
  std::string l_cyan = "\033[01;36m";
  std::string gray = "\033[22;37m";
  std::string white = "\033[01;37m";

  for (int i{}; i != h; ++i)
  {
    for (int j{}; j != w; ++j)
    {
      D x = x_start + D(j)*dx; // current real value
      D y = y_fin - D(i)*dy; // current imaginary value

      int const value = mandelbrot(x,y);

      if (value == 100) {std::cout << " ";}
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
    }
  }

  return 0;
}
