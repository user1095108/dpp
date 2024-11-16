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

static constexpr auto max_iter{1000000llu};

int main(int const argc, char* argv[]) noexcept
{
  std::mt19937 engine{std::random_device()()};
  std::uniform_real_distribution<float> distribution({});

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

  std::vector<std::vector<bool>> buffer(h);
  for (auto& row: buffer) row.resize(w);

  D x{}, y{};

  for (std::size_t i{}; max_iter != i; ++i)
  {
    D x1, y1;

    if (auto const r(distribution(engine)); r < .01_d32)
    {
      x1 = 0_d32;
      y1 = .16_d32 * y;
    }
    else if (r <= .86_d32)
    {
      x1 = .85_d32 * x + .04_d32 * y;
      y1 = -.04_d32 * x + .85_d32 * y + 1.6_d32;
    }
    else if (r <= .93_d32)
    {
      x1 = .2_d32 * x - .26_d32 * y;
      y1 = .23_d32 * x + .22_d32 * y + 1.6_d32;
    }
    else
    {
      x1 = -.15_d32 * x + .28_d32 * y;
      y1 = .26_d32 * x + .24_d32 * y + .44_d32;
    }

    x = x1; y = y1;

    buffer[std::size_t(h * (1_d32 - y/11))]
      [std::size_t(w * (.5_d32 + x/11))] = true;
  }

  for (auto const& row: buffer)
    for (auto const c: row)
      std::cout << "\033[4" << (c ? '2' : '9') << "m ";

  std::cout << "\033[0m";

  return 0;
}
