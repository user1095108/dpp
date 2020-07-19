// https://github.com/dario-marvin/Mandelbrot
// https://solarianprogrammer.com/2013/02/28/mandelbrot-set-cpp-11/
#include <cstdlib>

#include <iostream>

#include "dpp.hpp"

//using D = float;
using D = dpp::d32;

constexpr auto limit = 100;

constexpr int mandelbrot(D const r, D const i) noexcept
{
  auto zr(r), zi(i);

  for (int j{}; j != limit; ++j)
  {
    if (auto const zr2(zr * zr), zi2(zi * zi); zr2 + zi2 > 4)
    {
      return j;
    }
    else
    {
      zr = zr2 - zi2 + r;
      zi = D(2) * zr * zi + i;
    }
  }

  return limit;
}

int main()
{
  auto const w(WEXITSTATUS(std::system("exit `tput cols`")));
  auto const h(WEXITSTATUS(std::system("exit `tput lines`")) - 1);

  D const x0(-2), y0(1);
  D const x1(1), y1(-1);

  auto const dx((x1 - x0) / D(w));
  auto const dy((y1 - y0) / D(h));

  auto y(y0 + dy / D(2));

  for (int i{}; i != h; ++i, y += dy)
  {
    auto x(x0 + dx / D(2));

    for (int j{}; j != w; ++j, x += dx)
    {
      auto const t(D(mandelbrot(x, y))/D(limit));
      auto const t2(t * t);

      auto const olt(D(1) - t);
      auto const olt2(olt * olt);

      int const c[]{
        int(D(9)*D(255)*(olt*t2*t)),
        int(D(15)*D(255)*(olt2*t2)),
        int(D("8.5")*D(255)*(olt2*olt*t))
      };

      std::cout << "\033[48;2;" <<
        c[0] << ";" << c[1] << ";" << c[2] << "m" <<
        " ";
    }
  }

  std::cout << "\033[0m";

  return 0;
}
