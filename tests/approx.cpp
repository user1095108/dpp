#include <iostream>
#include <numeric>

#include "../midpoint.hpp"

auto pow(auto const x, auto const e) noexcept ->
  std::remove_const_t<decltype(x)>
{
  return !e ? 1 : 1 == e ? x : (e % 2 ? x : 1) * pow(x * x, e / 2);
}

int main()
{
//using D = long double;
  using D = dpp::d128;

  using dpp::midpoint; using std::midpoint;

  constexpr auto iterations(100000);

  std::cout.precision(18);

  { // π using Continued fractions
    std::size_t i(80);
    D x(2 * i + 3);

    do { x = D(2 * i + 1) + D(i + 1) * D(i + 1) / x; } while (i--);

    std::cout << "Approximation of π: " << 4 / x << std::endl;
  }

  { // π using the Leibniz series
    D leibniz_pi{};

    std::size_t i(iterations - 1);
    do { leibniz_pi += D(i % 2 ? -1 : 1) / (2 * i + 1); } while (i--);

    std::cout << "Approximation of π: " << 4 * leibniz_pi << std::endl;
  }

  { // e using the series expansion
    D e_approximation{};
    D factorial(1);

    for (std::size_t i(1); i <= iterations; factorial *= ++i)
      e_approximation += 1 / factorial;

    std::cout << "Approximation of e: " << e_approximation << std::endl;
  }

  auto const approx([&](auto const f, D const x0 = 1) noexcept
    {
      D x, y{x0}, diff(FLT_MAX);

      for (;;)
      {
        y = f(x = y);

        if (auto const diffn(abs(y - x)); diffn < diff)
          diff = diffn; else break;
      }

      return midpoint(x, y);
    }
  );

  { // √2 using the Newton-Raphson method
    std::cout << "Approximation of √2: " <<
      approx([](auto const x){return midpoint(x, 2 / x);}) << std::endl;
  }

  { // the Golden Ratio (φ) using Newton-Raphson for √5
    auto const sqrt5{approx([](auto const x){return midpoint(x, 5 / x);})};

    std::cout << "Approximation of φ (Golden Ratio): " << midpoint(D(1), sqrt5) << std::endl;
  }

  { // ln(2) using the Gregory series
    D ln2{};

    std::size_t i(iterations);
    do { ln2 += D(i % 2 ? 1 : -1) / i; } while (--i);

    std::cout << "Approximation of ln(2): " << ln2 << std::endl;
  }

  { // Catalan's constant (G)
    D catalan{};

    for (std::size_t k{}; k != iterations; ++k)
      catalan += D(k % 2 ? -1 : 1) / ((2 * k + 1) * (2 * k + 1));

    std::cout << "Approximation of G (Catalan's constant): " << catalan << std::endl;
  }

  { // Apéry's constant (ζ(3))
    D apery{};

    for (std::size_t k{1}; k <= iterations; ++k) apery += 1 / (D(k) * k * k);

    std::cout << "Approximation of ζ(3) (Apéry's constant): " << apery << std::endl;
  }

  { // Liouville's constant
    D liouville{};
    std::uintmax_t factorial(1);

    for (std::size_t k{1}; k <= 10; factorial *= ++k)
      liouville += 1 / pow(D(10), factorial);

    std::cout << "Approximation of Liouville's constant: " << liouville << std::endl;
  }

  {
    D m;

    for (D l(1), h(2); m = midpoint(l, h), (m != l) && (m != h);)
      if (auto const mr(pow(m, 12)); mr < 2) l = m; else h = m;

    std::cout << "Approximation of Twelfth root of two: " << m << std::endl;
  }

  {
    std::cout << "Approximation of Plastic ratio: " <<
      approx([](auto const x){return (2*x*x*x + 1) / (3*x*x - 1);}) << std::endl;
    std::cout << "Approximation of Golden ratio: " <<
      approx([](auto const x){return (2*x + 1) / (x + 1);}) << std::endl;
    std::cout << "Approximation of Silver ratio: " <<
      approx([](auto const x){return (3*x + 1) / (x + 1);}) << std::endl;
    std::cout << "Approximation of Supergolden ratio: " <<
      approx([](auto const x){return (2*x*x*x + 1) / (3*x*x - x);}) << std::endl;
    std::cout << "Approximation of Supersilver ratio: " <<
      approx([](auto const x){return (2*x*x*x + 1) / (3*x*x - 2*x);}) << std::endl;
  }

  return 0;
}
