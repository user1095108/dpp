// https://github.com/dario-marvin/Mandelbrot
// https://solarianprogrammer.com/2013/02/28/mandelbrot-set-cpp-11/
#include <sys/ioctl.h> //ioctl() and TIOCGWINSZ
#include <unistd.h> // for STDOUT_FILENO

#include <iostream>

#include "dpp.hpp"

//using D = long double;
//using D = float;
using D = dpp::d32;

constexpr auto max_iter = 100;

constexpr int mandelbrot(D const cr, D const ci) noexcept
{
  auto zr(cr), zi(ci);

  int j{1};

  for (; max_iter != j; ++j)
  {
    if (auto const zr2(zr * zr), zi2(zi * zi); zr2 + zi2 <= 4)
    {
      zi = 2 * zr * zi + ci;
      zr = zr2 - zi2 + cr;
    }
    else
    {
      break;
    }
  }

  return j;
}

int main() noexcept
{
  int w, h;

  {
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);

    w = ws.ws_col;
    h = ws.ws_row - 1;
  }

  D x0(-2); D y(1.15);
  D const x1(1), y1(-1.15);

  auto const dx((x1 - x0) / w);
  auto const dy((y1 - y) / h);

  x0 += .5 * dx;
  y += .5 * dy;

  for (int i{}; h != i; ++i, y += dy)
  {
    auto x(x0);

    for (int j{}; w != j; ++j, x += dx)
    {
      auto const t(mandelbrot(x, y) / D(max_iter));
      auto const olt(1 - t);

      std::cout << "\033[48;2;" <<
        int(9 * 255 * (olt*t*t*t)) << ';' <<
        int(15 * 255 * (olt*olt*t*t)) << ';' <<
        int(8.5 * 255 * (olt*olt*olt*t)) << "m ";
    }
  }

  std::cout << "\033[0m";

  return 0;
}
