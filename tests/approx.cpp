#include <iostream>
#include <numeric>

#include "../midpoint.hpp"

int main()
{
//using D = long double;
  using D = dpp::d128;

  constexpr auto iterations(100000);

  std::cout.precision(18);

  { // π using the Leibniz series
    D leibniz_pi{};

    for (std::size_t i{}; i != iterations; ++i)
      leibniz_pi += D(i % 2 ? -1 : 1) / (2 * i + 1);

    std::cout << "Approximation of π: " << 4 * leibniz_pi << std::endl;
  }

  { // e using the series expansion
    D e_approximation{};
    D factorial(1);

    for (std::size_t i(1); i <= iterations; factorial *= ++i)
      e_approximation += 1 / factorial;

    std::cout << "Approximation of e: " << e_approximation << std::endl;
  }

  { // √2 using the Newton-Raphson method
    D sqrt2(1); // Initial guess for √2

    for (std::size_t i{}; i != iterations; ++i)
      sqrt2 = (sqrt2 + D(2) / sqrt2) / 2;

    std::cout << "Approximation of √2: " << sqrt2 << std::endl;
  }

  { // the Golden Ratio (φ) using Newton-Raphson for √5
    D sqrt5{1}; // Initial guess for √5

    for (std::size_t i{}; i != iterations; ++i)
      sqrt5 = (sqrt5 + D(5) / sqrt5) / 2;

    D golden_ratio = (D(1) + sqrt5) / D(2);

    std::cout << "Approximation of φ (Golden Ratio): " << golden_ratio << std::endl;
  }

  { // ln(2) using the Gregory series
    D ln2{};

    for (std::size_t i{1}; i <= iterations; ++i) ln2 += D(i % 2 ? 1 : -1) / i;

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
    D factorial(1);

    for (std::size_t k{1}; k <= 10; factorial *= ++k)
    {
      D power(10);
      for (std::size_t i{1}; i != factorial; ++i) power *= 10;

      liouville += 1 / power;
    }

    std::cout << "Approximation of Liouville's constant: " << liouville << std::endl;
  }

  {
    D l(1), h(2);

    for (;;)
    {
      using dpp::midpoint;
      using std::midpoint;

      auto const m(midpoint(l, h));

      if ((m == l) || (m == h)) break;

      D mr(1);
      for (std::size_t i{}; i != 12; ++i) mr *= m;

      if (mr < 2) l = m; else h = m;
    }

    std::cout << "Approximation of Twelfth root of two: " << l << std::endl;
  }

  {
    auto const approx([&](auto const f) noexcept
      {
        D x, y{1}, diff(FLT_MAX);

        for (;;)
        {
          y = f(x = y);

          if (auto const diffn(std::abs(y - x)); diffn < diff)
            diff = diffn; else break;
        }

        return x;
      }
    );

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
