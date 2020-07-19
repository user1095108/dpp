// https://github.com/dario-marvin/Mandelbrot
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
      if (auto const n(mandelbrot(x, y)); limit == n)
      {
        std::cout << " ";
      }
      else
      {
        auto const t(D(n)/D(limit));
        auto const t2(t * t);

        auto const olt(D(1) - t);
        auto const olt2(olt * olt);

        int const c[]{
          int(D(9)*olt*t2*t*D(255)),
          int(D(15)*olt2*t2*D(255)),
          int(D("8.5")*olt2*olt*t*D(255))
        };

        std::cout << "\033[38;2;" <<
          c[0] << ";" << c[1] << ";" << c[2] << "m" <<
          "o";
      }
    }
  }

  std::cout << "\033[0m";

  return 0;
}
