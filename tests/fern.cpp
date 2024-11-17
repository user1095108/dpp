// https://en.wikipedia.org/wiki/Barnsley_fern
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#else
#include <sys/ioctl.h> //ioctl() and TIOCGWINSZ
#include <unistd.h> // for STDOUT_FILENO
#endif

#include <iostream>
#include <random>

#include "../dpp.hpp"

using namespace dpp::literals;

//using D = long double;
//using D = float;
using D = dpp::d32;

struct part_t
{
  D t[6];
  D p;
} const parts[][4]{
  {
    {{0_d32, 0_d32, 0_d32, .16_d32, 0_d32, 0_d32}, .01_d32},
    {{.85_d32, .04_d32, -.04_d32, .85_d32, 0_d32, 1.6_d32}, .85_d32},
    {{.2_d32, -.26_d32, .23_d32, .22_d32, 0_d32, 1.6_d32}, .07_d32},
    {{-.15_d32, .28_d32, .26_d32, .24_d32, 0_d32, .44_d32}, .07_d32}
  },
  {
    {{0_d32, 0_d32, 0_d32, .25_d32, 0_d32, -.4_d32}, .02_d32},
    {{.95_d32, .005_d32, -.005_d32, .93_d32, -.002_d32, .5_d32}, .84_d32},
    {{.035_d32, -.2_d32, .16_d32, .04_d32, -.09_d32, .02_d32}, .07_d32},
    {{-.04_d32, .2_d32, .16_d32, .04_d32, .083_d32, .12_d32}, .07_d32},
  },
  {
    {{0_d32, 0_d32, 0_d32, .2_d32, 0_d32, -.12_d32}, .01_d32},
    {{.845_d32, .035_d32, -.035_d32, .82_d32, 0_d32, 1.6_d32}, .85_d32},
    {{.2_d32, -.31_d32, .255_d32, .245_d32, 0_d32, .29_d32}, .07_d32},
    {{-.15_d32, .24_d32, .25_d32, .2_d32, 0_d32, .68_d32}, .07_d32}
  },
  {
    {{0_d32, 0_d32, 0_d32, .25_d32, 0_d32, -.14_d32}, .02_d32},
    {{.85_d32, .02_d32, -.02_d32, .83_d32, 0_d32, 1_d32}, .84_d32},
    {{.09_d32, -.28_d32, .3_d32, .11_d32, 0_d32, .6_d32}, .07_d32},
    {{-.09_d32, .28_d32, .3_d32, .09_d32, 0_d32, .7_d32}, .07_d32}
  },
  {
    {{0_d32, 0_d32, 0_d32, .16_d32, 0_d32, 0_d32}, .01_d32},
    {{.85_d32, .04_d32, -.04_d32, .85_d32, 0_d32, 1.6_d32}, .85_d32},
    {{.2_d32, -.26_d32, .23_d32, .22_d32, 0_d32, 1.6_d32}, .07_d32},
    {{-.15_d32, .28_d32, .26_d32, .24_d32, 0_d32, .44_d32}, .07_d32}
  },
  {
    {{.98_d32, -.97_d32, .09_d32, .96_d32, 0_d32, 1.7_d32}, .86_d32},
    {{.85_d32, .04_d32, -.04_d32, .85_d32, 0_d32, 1.6_d32}, .85_d32},
    {{.2_d32, -.26_d32, .23_d32, .22_d32, 0_d32, 1.6_d32}, .07_d32},
    {{-.15_d32, .28_d32, .26_d32, .24_d32, 0_d32, .44_d32}, .14_d32}
  },
  {
    {{0_d32, 0_d32, 0_d32, 0_d32, 0_d32, 0_d32}, 0_d32},
    {{.98_d32, -.97_d32, .09_d32, .96_d32, 0_d32, 1.7_d32}, .86_d32},
    {{0_d32, 0_d32, 0_d32, 0_d32, 0_d32, 0_d32}, 0_d32},
    {{-.15_d32, .28_d32, .2_d32, .24_d32, 0_d32, .44_d32}, .14_d32}
  },
};

static constexpr std::size_t max_iter{1000000u};

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

  auto xmax(D::min), xmin(D::max);
  auto ymax(D::min), ymin(D::max);

  std::vector<std::pair<D, D>> points;

  {
    part_t const* pr;

    if (2 == argc)
    {
      auto const tmp(std::atoi(argv[1]));

      pr = tmp < std::size(parts) ? parts[tmp] : *parts;
    }
    else
    {
      pr = *parts;
    }

    std::mt19937_64 engine{std::random_device()()};
    std::uniform_real_distribution<float> distribution({});

    D x{}, y{};

    for (std::size_t i{}; max_iter != i; ++i)
    {
      D r(distribution(engine)), x1, y1;

      for (std::size_t j{}; 4 != j; ++j)
      {
        if (auto const p(pr[j]); (3 == j) || (r < p.p))
        {
          x1 = p.t[0] * x + p.t[1] * y + p.t[4];
          y1 = p.t[2] * x + p.t[3] * y + p.t[5];

          break;
        }
        else
        {
          r -= p.p;
        }
      }

      x = x1; y = y1;

      if (x < xmin) xmin = x; if (x > xmax) xmax = x;
      if (y < ymin) ymin = y; if (y > ymax) ymax = y;

      points.push_back({x, y});
    }
  }

  std::cout << "\033[2J\033[42m";

  auto const dx(xmax - xmin), dy(ymax - ymin);
  auto const s(std::max(dx, dy)), sx(D(w - 1) / s), sy(D(h - 1) / s);
  auto const mx((xmax + xmin) / 2), my((ymax + ymin) / 2);

  for (auto const& p: points)
  {
    auto const& x(std::get<0>(p)), y(std::get<1>(p));

    std::cout << "\033[" <<
      std::to_string(int((my - y) * sy + D(h) / 2) + 1) << ';' <<
      std::to_string(int((x - mx) * sx + D(w) / 2) + 1) << "H ";
  }

  std::cout << "\033[0m\033[" << std::to_string(h) << ";1H";

  return 0;
}
