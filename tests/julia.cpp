#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#else
#include <sys/ioctl.h> // ioctl() and TIOCGWINSZ
#include <unistd.h> // STDOUT_FILENO
#endif

#include <cstdlib> // std::atoi
#include <iostream>

#include "../dpp.hpp"

using namespace dpp::literals;

//using D = long double;
//using D = float;
using D = dpp::d32;

static constexpr auto max_iter = 100u;

constexpr auto julia(D zr, D zi, D const cr, D const ci) noexcept
{
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

int main(int const argc, char* argv[]) noexcept
{
  int w, h;

  {
    #if defined(_WIN32)
      auto const handle(GetStdHandle(STD_OUTPUT_HANDLE));

      if (DWORD mode; GetConsoleMode(handle, &mode))
        SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

      if (CONSOLE_SCREEN_BUFFER_INFO csbi{};
        GetConsoleScreenBufferInfo(handle, &csbi))
      {
        w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        h = csbi.srWindow.Bottom - csbi.srWindow.Top;
      }
      else
        w = 80, h = 24; // sane fallback
    #else
      if (struct winsize ws{}; -1 != ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws))
      {
        w = ws.ws_col;
        h = ws.ws_row - 1;
      }
      else
        w = 80, h = 24; // sane fallback
    #endif

    if (1 > w) w = 1;
    if (1 > h) h = 1;
  }

  D a, b;

  if (2 == argc)
  {
    switch (std::atoi(argv[1]))
    { // https://en.wikipedia.org/wiki/Julia_set#Quadratic_polynomials
      default:
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

      case 7: // c = 1 - phi, phi = golden ratio (~1.618033989); real-valued
        a = -.61803_d32; b = {};
        break;

      case 8: // c = (phi - 2) + (phi - 1)i, approx. -0.4 + 0.6i
        a = -.4_d32; b = .6_d32;
        break;

      case 9: // c = 0.285 (real-valued)
        a = .285_d32; b = {};
        break;

      case 10: // c = -0.70176 - 0.3842i
        a = -.70176_d32; b = -.3842_d32;
        break;

      case 11: // c = -0.8 + 0.156i
        a = -.8_d32; b = .156_d32;
        break;

      case 12: // c = -0.7269 + 0.1889i
        a = -.7269_d32; b = .1889_d32;
        break;

      case 13: // c = 0.35 + 0.35i
        a = .35_d32; b = .35_d32;
        break;

      case 14: // c = 0.4 + 0.4i
        a = .4_d32; b = .4_d32;
        break;

      // The following four are the classic named quadratic Julia sets
      // (Weisstein, "Julia Set," MathWorld)

      case 15: // Dendrite fractal: c = i; a Misiurewicz point where the
               // Julia set degenerates into a branched, thread-like curve
        a = {}; b = 1_d32;
        break;

      case 16: // Douady's rabbit fractal: c = -0.123 + 0.745i
        a = -.123_d32; b = .745_d32;
        break;

      case 17: // San Marco fractal: c = -0.75 (real-valued); named for its
               // resemblance to the domes of St. Mark's Basilica in Venice
        a = -.75_d32; b = {};
        break;

      case 18: // Siegel disk fractal: c = -0.39054 - 0.58679i; contains a
               // Siegel disk, an invariant region on which the map acts
               // as an irrational rotation
        a = -.39054_d32; b = -.58679_d32;
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
