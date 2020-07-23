#include <sys/ioctl.h> //ioctl() and TIOCGWINSZ
#include <unistd.h> // for STDOUT_FILENO

#include <iostream>

#include "dpp.hpp"

//using D = long double;
//using D = float;
using D = dpp::d32;

constexpr auto max_iter = 100;

constexpr int julia(D const r, D const i, D const cr, D const ci) noexcept
{
  auto zr(r), zi(i);

  int j{};

  for (; j != max_iter; ++j)
  {
    if (auto const zr2(zr * zr), zi2(zi * zi); zr2 + zi2 <= 4)
    {
      zi = D(2) * zr * zi + ci;
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

  D a, b;

  if (2 == argc)
  {
    switch (std::atoi(argv[2]))
    {
      default:
      case 6:
        a= D(-.835); b = D(-.2321);
        break;

      case 1:
        a = D(-.7); b = D(.27015);
        break;

      case 2:
        a = D(.285); b = D(.01);
        break;

      case 3:
        a = D(-.74543); b = D(.11301);
        break;

      case 4:
        a = D(-.11); b = D(.6557);
        break;

      case 5:
        a = D(.45); b = D(.1428);
        break;
    }
  }
  else
  {
    a= D(-.835); b = D(-.2321);
  }

  D const x0(-1.6), y0(1.15);
  D const x1(1.6), y1(-1.15);

  auto const dx((x1 - x0) / D(w));
  auto const dy((y1 - y0) / D(h));

  auto y(y0 + D(.5) * dy);

  for (int i{}; i != h; ++i, y += dy)
  {
    auto x(x0 + D(.5) * dx);

    for (int j{}; j != w; ++j, x += dx)
    {
      auto const t(D(julia(x, y, a, b)) / D(max_iter));
      auto const olt(D(1) - t);

      std::cout << "\033[48;2;" <<
        int(D(9)*D(255)*(olt*t*t*t)) << ';' <<
        int(D(15)*D(255)*(olt*olt*t*t)) << ';' <<
        int(D(8.5)*D(255)*(olt*olt*olt*t)) << "m ";
    }
  }

  std::cout << "\033[0m";

  return 0;
}
