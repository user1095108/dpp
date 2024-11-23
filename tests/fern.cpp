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
  { // Barnsley fern - 0
    {{0, 0, 0, .16, 0, 0}, .01},
    {{.85, .04, -.04, .85, 0, 1.6}, .85},
    {{.2, -.26, .23, .22, 0, 1.6}, .07},
    {{-.15, .28, .26, .24, 0, .44}, .07}
  },
  { // Cyclosorus - 1
    {{0, 0, 0, .25, 0, -.4}, .02},
    {{.95, .005, -.005, .93, -.002, .5}, .84},
    {{.035, -.2, .16, .04, -.09, .02}, .07},
    {{-.04, .2, .16, .04, .083, .12}, .07},
  },
  { // Modified Barnsley fern - 2
    {{0, 0, 0, .2, 0, -.12}, .01},
    {{.845, .035, -.035, .82, 0, 1.6}, .85},
    {{.2, -.31, .255, .245, 0, .29}, .07},
    {{-.15, .24, .25, .2, 0, .68}, .07}
  },
  { // Culcita - 3
    {{0, 0, 0, .25, 0, -.14}, .02},
    {{.85, .02, -.02, .83, 0, 1}, .84},
    {{.09, -.28, .3, .11, 0, .6}, .07},
    {{-.09, .28, .3, .09, 0, .7}, .07}
  },
  { // Fishbone - 4
    {{0, 0, 0, .25, 0, -.4}, .02},
    {{.95, .002, -.002, .93, -.002, .5}, .84},
    {{.035, -.11, .27, .01, -.05, .005}, .07},
    {{-.04, .11, .27, .01, .047, .06}, .07}
  },
  { // 5, https://www.youtube.com/watch?v=45nWwtf5iOs
    {{0, 0, 0, 0, 0, 0}, 0.01},
    {{.98, -.97, .09, .96, 0, 1.7}, .85},
    {{0, 0, 0, 0, 0, 0}, 0.07},
    {{-.15, .28, .26, .24, 0, .44}, .07}
  },
  { // Hexapod - 6, https://www.youtube.com/watch?v=aSw5EMS923M
    {{0, 0, 0, 0, 0, 0}, 0},
    {{.98, -.97, .9, .96, 0, 1.7}, .86},
    {{0, 0, 0, 0, 0, 0}, 0},
    {{-.15, .28, .26, .24, 0, .44}, .14}
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
      h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    #else
      struct winsize ws;
      ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);

      w = ws.ws_col;
      h = ws.ws_row;
    #endif
  }

  D xmax{}, xmin{}, ymax{}, ymin{};

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

      for (auto p(pr); &pr[std::size(*parts)] != p; ++p) // p points to part
      {
        if ((&pr[std::size(*parts) - 1] == p) || (r < p->p))
        {
          x1 = p->t[0] * x + p->t[1] * y + p->t[4];
          y1 = p->t[2] * x + p->t[3] * y + p->t[5];

          break;
        }
        else
        {
          r -= p->p;
        }
      }

      x = x1; y = y1;

      if (x < xmin) xmin = x; else if (x > xmax) xmax = x; // xmin <= xmax
      if (y < ymin) ymin = y; else if (y > ymax) ymax = y; // ymin <= ymax

      points.emplace_back(x, y);
    }
  }

  std::vector<std::vector<bool>> buffer(--h);
  for (auto& l: buffer) l.resize(w);

  auto const mx((xmax + xmin) / 2), my((ymax + ymin) / 2);
  auto const sx(w / (xmax - xmin)), sy(h / (ymax - ymin));
  auto const hw(D(w - 1) / 2), hh(D(h - 1) / 2);

  for (auto const& [x, y]: points)
    buffer[std::size_t((my - y) * sy + hh)]
      [std::size_t((x - mx) * sx + hw)] = true;

  std::cout << "\033[49m";

  for (bool prevc{}; auto const& l: buffer)
  {
    for (auto const c: l)
    {
      if (prevc != c)
      {
        prevc = c;
        std::cout << "\033[4" << (c ? '2' : '9') << 'm';
      }

      std::cout << ' ';
    }
  }

  std::cout << "\033[0m";

  return 0;
}
