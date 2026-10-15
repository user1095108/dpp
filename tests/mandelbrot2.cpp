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

#include <algorithm> // std::min, std::max
#include <cstdlib> // std::atoi
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
        SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

      if (CONSOLE_SCREEN_BUFFER_INFO csbi{};
        GetConsoleScreenBufferInfo(handle, &csbi))
        w = csbi.srWindow.Right - csbi.srWindow.Left + 1,
        h = csbi.srWindow.Bottom - csbi.srWindow.Top;
      else
        w = 80, h = 24; // sane fallback
    #else
      if (struct winsize ws{}; -1 != ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws))
        w = ws.ws_col, h = ws.ws_row - 1;
      else
        w = 80, h = 24; // sane fallback
    #endif

    if (1 > w) w = 1;
    if (1 > h) h = 1;
  }

  D const ymax(1.15_d32), ymin(-1.15_d32);
  D const math_h = ymax - ymin;
  D const math_w = math_h * D(w) / D(2 * h);
  D const xmin  = -0.5_d32 - math_w / D(2);

  D const dx = math_w / w;
  D const dy = (ymin - ymax) / D(h);

  D y(ymax + dy / D(2));

  for (int i{}; h != i; ++i, y += dy)
  {
    D x(xmin + dx / D(2));

    for (int j{}; w != j; ++j, x += dx)
    {
      auto const it(mandelbrot(x, y));

      if (it == max_iter)
        std::cout << "\033[48;2;0;0;0m ";
      else
      {
        D const t(D(it) / D(max_iter));

        std::cout << "\033[48;2;"
          << int(255 * std::min(D(1), t * D(3))) << ';'
          << int(255 * std::max(D{}, std::min(D(1), (t - D(0.33)) * D(2.5)))) << ';'
          << int(255 * std::max(D{}, std::min(D(1), (t - D(0.66)) * D(3)))) << "m ";
      }
    }
  }

  std::cout << "\033[0m";
}
