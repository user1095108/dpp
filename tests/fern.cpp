// https://en.wikipedia.org/wiki/Barnsley_fern
// Additional IFS fractals from:
//   - Paul Bourke (paulbourke.net/fractals/ifs/)
//   - Larry Riddle (larryriddle.agnesscott.org/ifs/)
//   - Wikipedia (en.wikipedia.org/wiki/Lévy_C_curve)
//   - arXiv:2311.10102 (Twindragon, Golden Dragon)
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#else
#include <sys/ioctl.h> //ioctl() and TIOCGWINSZ
#include <unistd.h> // for STDOUT_FILENO
#endif

#include <iostream>
#include <numeric>
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
  { // Barnsley fern - 0, classic fern shape by Michael Barnsley (1988)
    // https://en.wikipedia.org/wiki/Barnsley_fern
    {{0, 0, 0, .16, 0, 0}, .01},
    {{.85, .04, -.04, .85, 0, 1.6}, .85},
    {{.2, -.26, .23, .22, 0, 1.6}, .07},
    {{-.15, .28, .26, .24, 0, .44}, .07}
  },
  { // Cyclosorus - 1, a delicate fern variant with tighter leaf curling
    // Parameters from Barnsley's original collection of fern mutants
    {{0, 0, 0, .25, 0, -.4}, .02},
    {{.95, .005, -.005, .93, -.002, .5}, .84},
    {{.035, -.2, .16, .04, -.09, .02}, .07},
    {{-.04, .2, .16, .04, .083, .12}, .07},
  },
  { // Modified Barnsley fern - 2, a stockier fern with broader leaflets
    // Variant with adjusted stem/leaflet ratios for denser appearance
    {{0, 0, 0, .2, 0, -.12}, .01},
    {{.845, .035, -.035, .82, 0, 1.6}, .85},
    {{.2, -.31, .255, .245, 0, .29}, .07},
    {{-.15, .24, .25, .2, 0, .68}, .07}
  },
  { // Culcita - 3, a compact fern species with rounded leaflets
    // Named after the genus of tree ferns; shorter and more rounded
    {{0, 0, 0, .25, 0, -.14}, .02},
    {{.85, .02, -.02, .83, 0, 1}, .84},
    {{.09, -.28, .3, .11, 0, .6}, .07},
    {{-.09, .28, .3, .09, 0, .7}, .07}
  },
  { // Fishbone - 4, a skeletal fern resembling a fish skeleton
    // Very narrow leaflets arranged along a prominent central rachis
    {{0, 0, 0, .25, 0, -.4}, .02},
    {{.95, .002, -.002, .93, -.002, .5}, .84},
    {{.035, -.11, .27, .01, -.05, .005}, .07},
    {{-.04, .11, .27, .01, .047, .06}, .07}
  },
  { // Sierpinski triangle - 5, https://en.wikipedia.org/wiki/Sierpinski_triangle
    // Classic self-similar triangle with three corner contractions
    {{0, 0, 0, 0, 0, 0}, 0},
    {{.5, 0, 0, .5, 0, 0}, .3333333},
    {{.5, 0, 0, .5, .5, 0}, .3333333},
    {{.5, 0, 0, .5, .25, .4330127}, .3333333}
  },
  { // Koch curve - 6, https://en.wikipedia.org/wiki/Koch_snowflake
    // One side of the Koch snowflake; four affine copies at 1/3 scale
    {{.3333333, 0, 0, .3333333, 0, 0}, .25},
    {{.1666667, -.2886751, .2886751, .1666667, .3333333, 0}, .25},
    {{.1666667, .2886751, -.2886751, .1666667, .5, .2886751}, .25},
    {{.3333333, 0, 0, .3333333, .6666667, 0}, .25}
  },
  { // Heighway dragon curve - 7, https://en.wikipedia.org/wiki/Dragon_curve
    // Space-filling curve discovered by NASA physicists; two rotated copies
    {{0, 0, 0, 0, 0, 0}, 0},
    {{.5, -.5, .5, .5, 0, 0}, .5},
    {{0, 0, 0, 0, 0, 0}, 0},
    {{-.5, -.5, .5, -.5, 1, 0}, .5}
  },
  { // Fractal tree - 8, a symmetric binary tree with trunk and branches
    // Classic L-system like tree using four affine transformations
    {{0, 0, 0, .5, 0, 0}, .05},
    {{.42, -.42, .42, .42, 0, .2}, .4},
    {{.42, .42, -.42, .42, 0, .2}, .4},
    {{.1, 0, 0, .1, 0, .2}, .15}
  },
  { // Maple leaf - 9, Canadian maple leaf by Paul Bourke (Jan 2002)
    // Four affine maps producing the iconic lobed leaf silhouette
    // Source: https://paulbourke.net/fractals/ifs/
    {{0.14, 0.01, 0.00, 0.51, -0.08, -1.31}, .25},
    {{0.43, 0.52, -0.45, 0.50, 1.49, -0.75}, .25},
    {{0.45, -0.49, 0.47, 0.47, -1.62, -0.74}, .25},
    {{0.49, 0.00, 0.00, 0.51, 0.02, 1.62}, .25}
  },
  { // Generic leaf - 10, an elm-like leaf by Paul Bourke (Jan 2002)
    // Asymmetric leaf with serrated edges and prominent midrib
    // Source: https://paulbourke.net/fractals/ifs/
    {{0.0000, 0.2439, 0.0000, 0.3053, 0.0000, 0.0000}, .25},
    {{0.7248, 0.0337, -0.0253, 0.7426, 0.2060, 0.2538}, .25},
    {{0.1583, -0.1297, 0.3550, 0.3676, 0.1383, 0.1750}, .25},
    {{0.3386, 0.3694, 0.2227, -0.0756, 0.0679, 0.0826}, .25}
  },
  { // Lévy C curve - 11, https://en.wikipedia.org/wiki/Lévy_C_curve
    // Self-similar curve by Paul Lévy (1938); two rotations by ±45°
    // Source: Larry Riddle, https://larryriddle.agnesscott.org/ifs/levy/LevyCode.htm
    {{0.5, -0.5, 0.5, 0.5, 0.0, 0.0}, .5},
    {{0.5, 0.5, -0.5, 0.5, 0.5, 0.5}, .5},
    {{0, 0, 0, 0, 0, 0}, 0},
    {{0, 0, 0, 0, 0, 0}, 0}
  },
  { // Twindragon - 12, a plane-filling fractal tile related to the dragon curve
    // Two copies scaled by 1/√2, rotated 45°; translation to (0.5, -0.5)
    // Source: Larry Riddle, https://larryriddle.agnesscott.org/ifs/heighway/twindragon.htm
    //         arXiv:2604.05010 (aspect ratio of the Twin Dragon)
    {{0.5, -0.5, 0.5, 0.5, 0.0, 0.0}, .5},
    {{0.5, -0.5, 0.5, 0.5, 0.5, -0.5}, .5},
    {{0, 0, 0, 0, 0, 0}, 0},
    {{0, 0, 0, 0, 0, 0}, 0}
  },
  { // Golden dragon - 13, a dragon curve with golden-ratio scaling
    // Scaling factors r and r² where r = 1/φ^(1/φ) ≈ 0.742; 
    // angles 32.894° and 133.014°. Probabilities weighted by r^d, r^(2d).
    // Source: Larry Riddle, https://larryriddle.agnesscott.org/ifs/heighway/goldenDragon.htm
    //         arXiv:2311.10102 (Mechanical Attributes of Fractal Dragons)
    {{0.62367, -0.40337, 0.40337, 0.62367, 0.0, 0.0}, .618034},
    {{-0.37633, -0.40337, 0.40337, -0.37633, 1.0, 0.0}, .381966},
    {{0, 0, 0, 0, 0, 0}, 0},
    {{0, 0, 0, 0, 0, 0}, 0}
  },
  { // Crystal - 14, symmetric lattice crystal fractal by Paul Bourke
    // A 4-map IFS creating an intricate, symmetric crystalline lattice structure
    // Source: https://paulbourke.net/fractals/ifs/
    {{0.387, 0.430, 0.430, -0.387, 0.2560, 0.5220}, .25},
    {{0.441, -0.091, -0.009, -0.322, 0.4219, 0.5059}, .25},
    {{-0.468, 0.020, -0.113, 0.015, 0.4000, 0.4000}, .25},
    {{0.400, -0.020, 0.041, 0.341, 0.0410, 0.0410}, .25}
  },
  { // Pentadentrite - 15, pentagonal self-similar fractal curve by Larry Riddle
    // A 5-fold self-similar fractal generated with transformations centered around 1/sqrt(5) scaling
    // Source: https://larryriddle.agnesscott.org/ifs/pentaden/pentaden.htm
    {{0.4472, -0.0, 0.0, 0.4472, 0.0, 0.0}, .25},
    {{0.3618, -0.2629, 0.2629, 0.3618, 0.4472, 0.0}, .25},
    {{0.1382, -0.4253, 0.4253, 0.1382, 0.8090, 0.2629}, .25},
    {{-0.2236, -0.3873, 0.3873, -0.2236, 0.9472, 0.6882}, .25}
  },
  { // Terdragon / Davis-Knuth Dragon - 16, 3-fold symmetric dragon curve
    // Self-similar curve made by replacing line segments with three smaller segments at 30° angles
    // Source: https://larryriddle.agnesscott.org/ifs/heighway/terdragon.htm
    {{0.5, -0.288675, 0.288675, 0.5, 0.0, 0.0}, .333333},
    {{0.0, 0.57735, -0.57735, 0.0, 0.5, 0.288675}, .333333},
    {{0.5, -0.288675, 0.288675, 0.5, 0.5, -0.288675}, .333334},
    {{0, 0, 0, 0, 0, 0}, 0}
  },
  { // Spiral / Swirl - 17, logarithmic swirling spiral by Paul Bourke
    // Two affine maps producing an intricate logarithmic spiral vortex
    // Source: https://paulbourke.net/fractals/ifs/
    {{0.745455, -0.459091, 0.406061, 0.887879, 1.460279, 0.691072}, .880435},
    {{-0.424242, -0.065152, -0.175758, -0.218182, 3.809568, 6.741477}, .119565},
    {{0, 0, 0, 0, 0, 0}, 0},
    {{0, 0, 0, 0, 0, 0}, 0}
  },
  { // Fractal Bush - 18, dense bushy plant structure by Paul Bourke
    // Asymmetric branching tree forming a dense canopy leaf pattern
    // Source: https://paulbourke.net/fractals/ifs/
    {{0.195, -0.026, 0.026, 0.195, 0, 0}, .05},
    {{0.462, 0.414, -0.252, 0.368, 0, 0.6}, .40},
    {{-0.058, -0.605, 0.605, -0.058, 0, 1.0}, .40},
    {{-0.035, 0.07, -0.469, 0.022, 0, 0.6}, .15}
  },
  { // Box Fractal - 19, 4-corner square tiling fractal by Larry Riddle
    // Four corner reductions at scale 1/3 forming a classic self-similar square grid
    // Source: https://larryriddle.agnesscott.org/ifs/box/box.htm
    {{0.333333, 0.0, 0.0, 0.333333, 0.0, 0.0}, .25},
    {{0.333333, 0.0, 0.0, 0.333333, 0.666667, 0.0}, .25},
    {{0.333333, 0.0, 0.0, 0.333333, 0.0, 0.666667}, .25},
    {{0.333333, 0.0, 0.0, 0.333333, 0.666667, 0.666667}, .25}
  },
  { // Dragon Flame - 20, flame-like spiral curve by Paul Bourke
    // Asymmetric two-transform system creating a tendril flame structure
    // Source: https://paulbourke.net/fractals/ifs/
    {{0.824074, 0.281482, -0.212346, 0.864198, -1.882290, -0.110607}, .787473},
    {{0.088272, 0.520988, -0.463889, -0.377778, 0.785360, 8.095795}, .212527},
    {{0, 0, 0, 0, 0, 0}, 0},
    {{0, 0, 0, 0, 0, 0}, 0}
  },
  { // Fractal Cross - 21, self-similar 4-map cross pattern
    // Composed of four sub-squares structured as a cross
    {{0.333333, 0, 0, 0.333333, 0, 0.666667}, .25},
    {{0.333333, 0, 0, 0.333333, 0.333333, 0.333333}, .25},
    {{0.333333, 0, 0, 0.333333, -0.333333, 0.333333}, .25},
    {{0.333333, 0, 0, 0.333333, 0, 0}, .25}
  },
  { // Sierpinski Right Triangle - 22, asymmetrical right-angled gasket
    // Based on the standard Sierpinski but mapped into a right triangle
    {{0.5, 0, 0, 0.5, 0, 0}, .333333},
    {{0.5, 0, 0, 0.5, 0.5, 0}, .333333},
    {{0.5, 0, 0, 0.5, 0, 0.5}, .333334},
    {{0, 0, 0, 0, 0, 0}, 0}
  },
  { // Coral - 23, organic branching structure resembling sea coral
    // Fluid, overlapping scaling elements based on 3 standard maps
    {{0.30, -0.11, 0.11, 0.30, 0.0, 0.16}, .333333},
    {{0.85, 0.06, -0.06, 0.85, 0.0, 0.16}, .333333},
    {{0.30, 0.09, -0.09, 0.30, 0.0, 0.16}, .333334},
    {{0, 0, 0, 0, 0, 0}, 0}
  },
  { // Paul Bourke's Alternative Tree - 24, intricate asymmetrical fractal tree
    // Complex interplay of scaling and heavy translations 
    // Source: https://paulbourke.net/fractals/ifs/
    {{0.195, -0.288, 0.344, 0.443, 0.4431, 0.2452}, .25},
    {{0.462, 0.414, -0.252, 0.361, 0.2511, 0.5692}, .25},
    {{-0.058, -0.070, 0.453, -0.111, 0.5976, 0.0969}, .25},
    {{-0.035, 0.070, -0.469, -0.022, 0.4884, 0.5069}, .25}
  },
  { // 3-Map Spiral - 25, another logarithmic spiral variant by Paul Bourke 
    // Requires three maps to draw out an elaborate inward pinwheel
    // Source: https://paulbourke.net/fractals/ifs/
    {{0.787879, -0.424242, 0.242424, 0.859848, 1.758647, 1.408065}, .90},
    {{-0.121212, 0.257576, 0.151515, 0.053030, -6.721654, 1.377236}, .05},
    {{0.181818, -0.136364, 0.090909, 0.181818, 6.086107, 1.568035}, .05},
    {{0, 0, 0, 0, 0, 0}, 0}
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

  D xmax, xmin, ymax, ymin;

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

      if (!i) xmin = xmax = x, ymin = ymax = y;
      else
      {
        if (x < xmin) xmin = x; else if (x > xmax) xmax = x; // xmin <= xmax
        if (y < ymin) ymin = y; else if (y > ymax) ymax = y; // ymin <= ymax
      }

      points.emplace_back(x, y);
    }
  }

  std::vector<std::vector<bool>> buffer(--h);
  for (auto& l: buffer) l.resize(w);

  auto const mx(midpoint(xmin, xmax)), my(midpoint(ymin, ymax));
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
