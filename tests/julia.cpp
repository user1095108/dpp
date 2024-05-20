#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#else
#include <sys/ioctl.h> //ioctl() and TIOCGWINSZ
#include <unistd.h> // for STDOUT_FILENO
#endif

#include <iostream>

#include "../dpp.hpp"

using namespace dpp::literals;

//using D = long double;
//using D = float;
using D = dpp::d32;

constexpr auto max_iter = 100u;

constexpr auto julia(D zr, D zi, D const cr, D const ci) noexcept
{
  unsigned j{};

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

int main(int const argc, char* argv[]) noexcept
{
  int w, h;

  {
    #if defined(_WIN32)
      auto const handle(GetStdHandle(STD_OUTPUT_HANDLE));

      if (DWORD mode; GetConsoleMode(handle, &mode))
      {
        SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
      }

      CONSOLE_SCREEN_BUFFER_INFO csbi;
      GetConsoleScreenBufferInfo(handle, &csbi);
      w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
      h = csbi.srWindow.Bottom - csbi.srWindow.Top;
    #else
      struct winsize ws;
      ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);

      w = ws.ws_col;
      h = ws.ws_row - 1;
    #endif
  }

  D a, b;

  if (2 == argc)
  {
    switch (std::atoi(argv[1]))
    { // https://en.wikipedia.org/wiki/Julia_set#Quadratic_polynomials
      default:
      case 7:
        a = -.835_d32; b = -.2321_d32;
        break;

      case 1:
        a = -.7_d32; b = .27015_d32;
        break;

      case 2:
        a = .285_d32; b = .01_d32;
        break;

      case 3:
        a = -.74543_d32; b = .11301_d32;
        break;

      case 4:
        a = -.11_d32; b = .6557_d32;
        break;

      case 5:
        a = .45_d32; b = .1428_d32;
        break;

      case 6:
        a = {}; b = -.8_d32;
        break;
    }
  }
  else
  {
    a = -.835_d32; b = -.2321_d32;
  }

  D x0(-1.6_d32), y(1.15_d32);
  D const x1(1.6_d32), y1(-1.15_d32);

  auto const dx((x1 - x0) / w);
  auto const dy((y1 - y) / h);

  x0 += .5_d32 * dx;
  y += .5_d32 * dy;

  for (int i{}; h != i; ++i, y += dy)
  {
    auto x(x0);

    for (int j{}; w != j; ++j, x += dx)
    {
      auto const t(julia(x, y, a, b) / D(max_iter));
      auto const olt(1 - t);

      std::cout << "\033[48;2;" <<
        int(9 * 255 * (olt*t*t*t)) << ';' <<
        int(15 * 255 * (olt*olt*t*t)) << ';' <<
        int(8.5_d32 * 255 * (olt*olt*olt*t)) << "m ";
    }
  }

  std::cout << "\033[0m";

  return 0;
}
