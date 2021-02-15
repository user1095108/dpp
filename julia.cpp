#include <sys/ioctl.h> //ioctl() and TIOCGWINSZ
#include <unistd.h> // for STDOUT_FILENO

#include <cstdlib>

#include <iostream>

#include "dpp.hpp"

//using D = long double;
//using D = float;
using D = dpp::d32;

constexpr auto max_iter = 100;

constexpr int julia(D zr, D zi, D const cr, D const ci) noexcept
{
  int j{};

  for (; j != max_iter; ++j)
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

int main(int const argc, char* argv[]) noexcept
{
  int w, h;

  {
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);

    w = ws.ws_col;
    h = ws.ws_row - 1;
  }

  D a, b;

  if (2 == argc)
  {
    switch (std::atoi(argv[1]))
    {
      default:
      case 6:
        a = -.835; b = -.2321;
        break;

      case 1:
        a = -.7; b = .27015;
        break;

      case 2:
        a = .285; b = .01;
        break;

      case 3:
        a = -.74543; b = .11301;
        break;

      case 4:
        a = -.11; b = .6557;
        break;

      case 5:
        a = .45; b = .1428;
        break;
    }
  }
  else
  {
    a = -.835; b = -.2321;
  }

  D x0(-1.6), y(1.15);
  D const x1(1.6), y1(-1.15);

  auto const dx((x1 - x0) / w);
  auto const dy((y1 - y) / h);

  x0 += .5 * dx;
  y += .5 * dy;

  for (int i{}; i != h; ++i, y += dy)
  {
    auto x(x0);

    for (int j{}; j != w; ++j, x += dx)
    {
      auto const t(julia(x, y, a, b) / D(max_iter));
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
