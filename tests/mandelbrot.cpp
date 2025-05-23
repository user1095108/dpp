// https://github.com/dario-marvin/Mandelbrot
// https://solarianprogrammer.com/2013/02/28/mandelbrot-set-cpp-11/
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

static constexpr auto max_iter = 100u;

constexpr auto mandelbrot(D const cr, D const ci) noexcept
{
  auto zr(cr), zi(ci);

  unsigned j{};

  for (; max_iter != j; ++j)
  {
    if (auto const zr2(zr * zr), zi2(zi * zi); zr2 + zi2 <= 4)
    {
      zi = fma(zr + zr, zi, ci); // zi = 2 * zr * zi + ci;
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

  D x0(-2_d32); D y(1.15_d32);
  D const x1(1_d32), y1(-1.15_d32);

  auto const dx((x1 - x0) / w);
  auto const dy((y1 - y) / h);

  x0 += .5_d32 * dx;
  y += .5_d32 * dy;

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
        int(8.5_d32 * 255 * (olt*olt*olt*t)) << "m ";
    }
  }

  std::cout << "\033[0m";

  return 0;
}
