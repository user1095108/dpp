#include <cassert>
#include "../dpp.hpp"
#include "../sqrt.hpp"
#include "../to_integral.hpp"
#include "../utils.hpp"

#include <cmath>
#include <compare>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

//////////////////////////////////////////////////////////////////////////////
// tiny test framework (global scope, no namespace)
//////////////////////////////////////////////////////////////////////////////
int g_failures = 0;
int g_checks   = 0;
int g_xfail_ok = 0;
int g_xpass    = 0;

#define CHECK(expr)                                                     \
  do {                                                                  \
    ++g_checks;                                                         \
    if (!(expr)) {                                                      \
      ++g_failures;                                                     \
      std::cerr << "FAIL  " << __FILE__ << ':' << __LINE__              \
                << "  " #expr "\n";                                     \
    }                                                                   \
  } while (0)

#define CHECK_EQ(a, b)                                                  \
  do {                                                                  \
    ++g_checks;                                                         \
    auto _va = (a); auto _vb = (b);                                     \
    if (!(_va == _vb)) {                                                \
      ++g_failures;                                                     \
      std::cerr << "FAIL  " << __FILE__ << ':' << __LINE__              \
                << "  " #a " == " #b "  ("                              \
                << _va << " vs " << _vb << ")\n";                       \
    }                                                                   \
  } while (0)

#define SECTION(name) std::cerr << "-- " name "\n"

using dpp::d16;
using dpp::d24;
using dpp::d32;
using dpp::d48;
using dpp::d64;
using dpp::d96;
using dpp::d128;
using dpp::d256;
using dpp::d512;
using dpp::d1024;
using dpp::direct;

using namespace dpp::literals;

//////////////////////////////////////////////////////////////////////////////
// construction
//////////////////////////////////////////////////////////////////////////////
void test_construction()
{
  SECTION("construction");

  {
    d32 a{};
    CHECK_EQ(a.sig(), 0);
    CHECK_EQ(a.exp(), 0);
    CHECK(!static_cast<bool>(a));
  }
  {
    d32 a(0);
    CHECK(!static_cast<bool>(a));

    d32 b(1);
    CHECK(static_cast<bool>(b));
    CHECK_EQ(b.sig(), 1);
    CHECK_EQ(b.exp(), 0);

    d32 c(12345);
    CHECK_EQ(c.sig(), 12345);
    CHECK_EQ(c.exp(), 0);

    d32 d(-12345);
    CHECK_EQ(d.sig(), -12345);
    CHECK_EQ(d.exp(), 0);
  }
  {
    d16 a(123456);
    CHECK_EQ(a.exp(), 1);
    CHECK_EQ(a.sig(), 12346);
  }
  {
    d32 b(0.0);
    CHECK(!static_cast<bool>(b));
  }
  {
    d32 a(dpp::nan);
    CHECK(dpp::isnan(a));
    CHECK(static_cast<bool>(a));

    d32 b(std::numeric_limits<double>::quiet_NaN());
    CHECK(dpp::isnan(b));
  }
  {
    d32 a(direct, 42, -1);
    CHECK_EQ(a.sig(), 42);
    CHECK_EQ(a.exp(), -1);
  }
  {
    d64 a(123);
    d32 b(a);
    CHECK_EQ(b.sig(), 123);
    CHECK_EQ(b.exp(), 0);

    d128 c(a);
    CHECK_EQ(c.sig(), 123);
    CHECK_EQ(c.exp(), 0);
  }
}

//////////////////////////////////////////////////////////////////////////////
// arithmetic
//////////////////////////////////////////////////////////////////////////////
void test_arithmetic()
{
  SECTION("arithmetic");

  {
    CHECK_EQ(d32(1) + d32(2), d32(3));
    CHECK_EQ(d32(1) + d32(-1), d32(0));
    CHECK_EQ(d32(100) + d32(23), d32(123));
    CHECK_EQ(d32(-5) + d32(-7), d32(-12));
  }
  {
    CHECK_EQ(d32(5) - d32(3), d32(2));
    CHECK_EQ(d32(1) - d32(1), d32(0));
    CHECK_EQ(d32(-5) - d32(3), d32(-8));
  }
  {
    CHECK_EQ(d32(3) * d32(4), d32(12));
    CHECK_EQ(d32(-3) * d32(4), d32(-12));
    CHECK_EQ(d32(100) * d32(100), d32(10000));
    CHECK_EQ(d32(0) * d32(5), d32(0));
  }
  {
    CHECK_EQ(d32(12) / d32(4), d32(3));
    CHECK_EQ(d32(-12) / d32(4), d32(-3));
    CHECK_EQ(d32(1) / d32(3),
             d32(0.3333333333333333333333333333333333));
  }
  {
    CHECK(dpp::isnan(d32(1) / d32(0)));
  }
  {
    auto r = d32(1) + d64(2);
    static_assert(std::is_same_v<decltype(r), d64>);
    CHECK_EQ(r, d64(3));
  }
  {
    d32 a(10);
    a += d32(5);  CHECK_EQ(a, d32(15));
    a -= d32(3);  CHECK_EQ(a, d32(12));
    a *= d32(2);  CHECK_EQ(a, d32(24));
    a /= d32(4);  CHECK_EQ(a, d32(6));
  }
  {
    d32 a(5);
    CHECK_EQ(++a, d32(6));
    CHECK_EQ(a++, d32(6));
    CHECK_EQ(a,   d32(7));
    CHECK_EQ(--a, d32(6));
    CHECK_EQ(a--, d32(6));
    CHECK_EQ(a,   d32(5));
  }
  {
    CHECK_EQ(+d32(5), d32(5));
    CHECK_EQ(-d32(5), d32(-5));
    CHECK_EQ(-d32(-5), d32(5));
  }
  {
    CHECK_EQ(d32(1) + 2, d32(3));
    CHECK_EQ(2 + d32(1), d32(3));
    CHECK_EQ(d32(1) * 2.0, d32(2));
    CHECK_EQ(2.0 * d32(1), d32(2));
    CHECK_EQ(d32(10) / 2, d32(5));
  }
  {
    CHECK(dpp::isnan(d32(dpp::nan) + d32(1)));
    CHECK(dpp::isnan(d32(1) + d32(dpp::nan)));
    CHECK(dpp::isnan(d32(dpp::nan) * d32(1)));
    CHECK(dpp::isnan(d32(dpp::nan) / d32(1)));
    CHECK(dpp::isnan(d32(1) / d32(dpp::nan)));
  }
}

//////////////////////////////////////////////////////////////////////////////
// comparisons
//////////////////////////////////////////////////////////////////////////////
void test_comparisons()
{
  SECTION("comparisons");

  {
    CHECK(d32(1) == d32(1));
    CHECK(!(d32(1) == d32(2)));
    CHECK(d32(1) != d32(2));
    CHECK(!(d32(1) != d32(1)));

    CHECK(!(d32(dpp::nan) == d32(dpp::nan)));
    CHECK(d32(dpp::nan) != d32(dpp::nan));
  }
  {
    CHECK(d32(1) < d32(2));
    CHECK(d32(2) > d32(1));
    CHECK(d32(1) <= d32(1));
    CHECK(d32(1) >= d32(1));
    CHECK(!(d32(1) < d32(1)));
    CHECK(!(d32(1) > d32(1)));
  }
  {
    CHECK(!(d32(dpp::nan) < d32(1)));
    CHECK(!(d32(dpp::nan) > d32(1)));
    CHECK(!(d32(dpp::nan) <= d32(1)));
    CHECK(!(d32(dpp::nan) >= d32(1)));
    CHECK(!(d32(1) < d32(dpp::nan)));
    CHECK(!(d32(1) > d32(dpp::nan)));
  }
  {
    CHECK(d32(dpp::nan) == dpp::nan);
    CHECK(dpp::nan == d32(dpp::nan));
    CHECK(d32(1) != dpp::nan);
    CHECK(dpp::nan != d32(1));
  }
  {
    CHECK(d32(1) == 1);
    CHECK(1 == d32(1));
    CHECK(d32(1) < 2);
    CHECK(1 < d32(2));
    CHECK(d32(2) > 1);
    CHECK(2 > d32(1));
  }
  {
    CHECK(d32(1) == d64(1));
    CHECK(d32(1) < d64(2));
    CHECK(d128(100) == d16(100));
  }
}

//////////////////////////////////////////////////////////////////////////////
// utilities
//////////////////////////////////////////////////////////////////////////////
void test_utilities()
{
  SECTION("utilities");

  {
    CHECK_EQ(dpp::abs(d32(5)), d32(5));
    CHECK_EQ(dpp::abs(d32(-5)), d32(5));
    CHECK(dpp::isnan(dpp::abs(d32(dpp::nan))));
  }
  {
    CHECK_EQ(dpp::trunc(d32(1.5)), d32(1));
    CHECK_EQ(dpp::trunc(d32(-1.5)), d32(-1));
    CHECK_EQ(dpp::trunc(d32(2)), d32(2));

    CHECK_EQ(dpp::floor(d32(1.5)), d32(1));
    CHECK_EQ(dpp::floor(d32(-1.5)), d32(-2));
    CHECK_EQ(dpp::floor(d32(2)), d32(2));

    CHECK_EQ(dpp::ceil(d32(1.5)), d32(2));
    CHECK_EQ(dpp::ceil(d32(-1.5)), d32(-1));
    CHECK_EQ(dpp::ceil(d32(2)), d32(2));

    CHECK_EQ(dpp::round(d32(1.4)), d32(1));
    CHECK_EQ(dpp::round(d32(1.5)), d32(2));
    CHECK_EQ(dpp::round(d32(-1.4)), d32(-1));
    CHECK_EQ(dpp::round(d32(-1.5)), d32(-2));
  }
  {
    CHECK_EQ(dpp::inv(d32(2)), d32(0.5));
    CHECK_EQ(dpp::inv(d32(4)), d32(0.25));
    CHECK(dpp::isnan(dpp::inv(d32(0))));
  }
  {
    CHECK_EQ(dpp::midpoint(d32(0), d32(10)), d32(5));
    CHECK_EQ(dpp::midpoint(d32(-10), d32(10)), d32(0));
  }
  {
    CHECK_EQ(dpp::fma(d32(2), d32(3), d32(4)), d32(10));
    CHECK_EQ(dpp::fma(d32(0), d32(3), d32(4)), d32(4));
    CHECK_EQ(dpp::fma(d32(-2), d32(3), d32(4)), d32(-2));
  }
}

//////////////////////////////////////////////////////////////////////////////
// conversions
//////////////////////////////////////////////////////////////////////////////
void test_conversions()
{
  SECTION("conversions");

  {
    CHECK_EQ(static_cast<double>(d32(1)),  1.0);
    CHECK_EQ(static_cast<double>(d32(1.5)), 1.5);
    CHECK_EQ(static_cast<double>(d32(-2.25)), -2.25);
    CHECK(std::isnan(static_cast<double>(d32(dpp::nan))));
  }
  {
    CHECK_EQ(static_cast<int>(d32(42)), 42);
    CHECK_EQ(static_cast<int>(d32(-42)), -42);
    CHECK_EQ(static_cast<int>(d32(42.9)), 42);
  }
  {
    CHECK(static_cast<bool>(d32(1)));
    CHECK(!static_cast<bool>(d32(0)));
    CHECK(static_cast<bool>(d32(dpp::nan)));
  }
  {
    CHECK_EQ(dpp::to_decimal<d32>("123"), d32(123));
    CHECK_EQ(dpp::to_decimal<d32>("-123"), d32(-123));
    CHECK_EQ(dpp::to_decimal<d32>("1.5"), d32(1.5));
    CHECK_EQ(dpp::to_decimal<d32>("-1.5"), d32(-1.5));
    CHECK_EQ(dpp::to_decimal<d32>("0"), d32(0));

    CHECK(dpp::isnan(dpp::to_decimal<d32>("")));
    CHECK(dpp::isnan(dpp::to_decimal<d32>("abc")));
  }
  {
    CHECK_EQ(dpp::to_string(d32(123)), std::string("123"));
    CHECK_EQ(dpp::to_string(d32(-123)), std::string("-123"));
    CHECK_EQ(dpp::to_string(d32(1.5)), std::string("1.5"));
    CHECK_EQ(dpp::to_string(d32(0.5)), std::string("0.5"));
    CHECK_EQ(dpp::to_string(d32(0)), std::string("0"));
    CHECK_EQ(dpp::to_string(d32(dpp::nan)), std::string("nan"));
  }
}

//////////////////////////////////////////////////////////////////////////////
// stream I/O
//////////////////////////////////////////////////////////////////////////////
void test_stream_io()
{
  SECTION("stream I/O");

  {
    std::ostringstream os;
    os << d32(123);
    CHECK_EQ(os.str(), std::string("123"));

    std::ostringstream os2;
    os2 << d32(1.5);
    CHECK_EQ(os2.str(), std::string("1.5"));

    std::ostringstream os3;
    os3 << d32(dpp::nan);
    CHECK_EQ(os3.str(), std::string("nan"));
  }
  {
    std::istringstream is("123");
    d32 a;
    is >> a;
    CHECK_EQ(a, d32(123));
    CHECK(!is.fail());
  }
  {
    std::istringstream is("-1.5");
    d32 a;
    is >> a;
    CHECK_EQ(a, d32(-1.5));
  }
  {
    std::istringstream is("nan");
    d32 a;
    is >> a;
    CHECK(dpp::isnan(a));
  }
  {
    std::istringstream is("abc");
    d32 a;
    is >> a;
    CHECK(is.fail());
  }
  {
    std::istringstream is("1.5 2.5");
    d32 a, b;
    is >> a >> b;
    CHECK(b == d32(2.5));
  }
}

//////////////////////////////////////////////////////////////////////////////
// special values
//////////////////////////////////////////////////////////////////////////////
void test_special_values()
{
  SECTION("special values");

  CHECK(dpp::isnan(d32(dpp::nan)));
  CHECK(!dpp::isnan(d32(0)));
  CHECK(!dpp::isnan(d32(1)));

  CHECK(!dpp::isnan(d32::eps));
  CHECK(!dpp::isnan(d32::max));
  CHECK(!dpp::isnan(d32::min));

  CHECK(static_cast<bool>(d32::eps));
  CHECK(static_cast<bool>(d32::max));
  CHECK(static_cast<bool>(d32::min));
}

//////////////////////////////////////////////////////////////////////////////
// literals
//////////////////////////////////////////////////////////////////////////////
void test_literals()
{
  SECTION("literals");

  using namespace dpp::literals;

  CHECK_EQ(123_d32, d32(123));
  CHECK_EQ(-123_d32, d32(-123));
  CHECK_EQ(1.5_d32, d32(1.5));
  CHECK_EQ(0_d64, d64(0));
  CHECK_EQ(42_d128, d128(42));
}

//////////////////////////////////////////////////////////////////////////////
// hashing
//////////////////////////////////////////////////////////////////////////////
void test_hashing()
{
  SECTION("hashing");

  std::hash<d32> h;

  CHECK_EQ(h(d32(123)), h(d32(123)));
  CHECK_EQ(h(d32(1.5)), h(d32(1.5)));
  CHECK_EQ(h(d32(-1.5)), h(d32(-1.5)));

  CHECK_EQ(h(d32(direct, 15, -1)), h(d32(direct, 150, -2)));
  CHECK_EQ(h(d32(0)), h(d32(direct, 0, 5)));

  CHECK_EQ(h(d32(dpp::nan)), h(d32(dpp::nan)));

  std::unordered_set<d32> s;
  s.insert(d32(1));
  s.insert(d32(2));
  s.insert(d32(1));
  CHECK_EQ(s.size(), std::size_t(2));
  CHECK(s.count(d32(1)) == 1);
  CHECK(s.count(d32(3)) == 0);
}

//////////////////////////////////////////////////////////////////////////////
// numeric
//////////////////////////////////////////////////////////////////////////////
void test_numeric()
{
  SECTION("numeric");

  // ---- exact integer arithmetic ----
  {
    CHECK_EQ(d32(999) * d32(999), d32(998001));
    CHECK_EQ(d64(12345) * d64(67890), d64(838102050));
    CHECK_EQ(d64(999999999LL) * d64(2), d64(1999999998LL));
    CHECK_EQ(d64(123456789LL) + d64(987654321LL), d64(1111111110LL));
    CHECK_EQ(d64(1LL << 40) / d64(1LL << 20), d64(1LL << 20));
  }

  // ---- exact decimal arithmetic ----
  {
    CHECK_EQ(d32(0.1) + d32(0.2), d32(0.3));
    CHECK_EQ(d32(1) / d32(8), d32(0.125));
    CHECK_EQ(d32(1) / d32(4), d32(0.25));
    CHECK_EQ(d32(3) * d32(0.5), d32(1.5));
    CHECK_EQ(d32(2.5) * d32(4), d32(10));
  }

  // ---- accumulating 0.1 ten times from 1.0 ----
  {
    d32 s(1);
    for (int i = 0; i < 10; ++i) s += d32(0.1);
    CHECK_EQ(s, d32(2));
  }

  // ---- accumulating 0.01 one hundred times from 0 ----
  {
    d32 s(0);
    for (int i = 0; i < 100; ++i) s += d32(0.01);
    CHECK_EQ(s, d32(1));
  }

  // ---- triangular numbers 1 + 2 + ... + 1000 = 500500 ----
  {
    d32 s(0);
    for (int i = 1; i <= 1000; ++i) s += d32(i);
    CHECK_EQ(s, d32(500500));
  }

  // ---- factorials ----
  {
    d128 f(1);
    for (int i = 2; i <= 20; ++i) f *= d128(i);
    CHECK_EQ(f, d128(2432902008176640000LL));

    d128 f25(1);
    for (int i = 2; i <= 25; ++i) f25 *= d128(i);
    CHECK_EQ(f25, 15511210043330985984000000_d128);
  }

  // ---- Fibonacci F(100) = 354224848179261915075 ----
  {
    d128 a(0), b(1);
    for (int i = 0; i < 100; ++i)
    {
      d128 t = a + b;
      a = b;
      b = t;
    }
    CHECK_EQ(a, 354224848179261915075_d128);
  }

  // ---- powers of 2 ----
  {
    d128 p(1);
    for (int i = 0; i < 100; ++i) p *= d128(2);
    CHECK_EQ(p, 1267650600228229401496703205376_d128);

    d256 q(1);
    for (int i = 0; i < 200; ++i) q *= d256(2);
    CHECK_EQ(q,
      1606938044258990275541962092341162602522202993782792835301376_d256);
  }

  // ---- geometric series: sum_{i=1}^{50} 1/2^i = 1 - 2^-50 ----
  {
    d128 s(0), term(1);
    for (int i = 0; i < 50; ++i)
    {
      term = term / 2;
      s = s + term;
    }
    d128 diff = d128(1) - s;
    CHECK(diff > d128(0));
    CHECK(diff < .000000000000001_d128);
  }

  // ---- Newton's method for sqrt(2) ----
  {
    d128 x(1.5);
    for (int i = 0; i < 100; ++i)
      x = (x + d128(2) / x) / 2;

    d128 err = dpp::abs(x * x - d128(2));
    CHECK(err < .0000000000000000000000000000000001_d128);
    CHECK(x > 1.414213562373_d128);
    CHECK(x < 1.414213562374_d128);
  }

  // ---- compound interest: 1.05^10 ≈ 1.6288946268 ----
  {
    d128 p(1);
    for (int i = 0; i < 10; ++i) p *= 1.05_d128;
    CHECK(p > 1.628_d128);
    CHECK(p < 1.629_d128);
  }

  // ---- harmonic number H_100 ≈ 5.18737751763962 ----
  {
    d128 h(0);
    for (int i = 1; i <= 100; ++i) h += d128(1) / d128(i);
    CHECK(h > 5.18737_d128);
    CHECK(h < 5.18738_d128);
  }

  // ---- pi via Machin's formula with d256 ----
  {
    auto arctan_recip = [](int n, int terms) -> d256
    {
      d256 sum(0);
      d256 n2  = d256(n) * d256(n);
      d256 term = d256(1) / d256(n);
      for (int k = 0; k < terms; ++k)
      {
        d256 contrib = term / d256(2 * k + 1);
        sum = sum + contrib;
        term = -term / n2;
      }
      return sum;
    };

    d256 pi = d256(16) * arctan_recip(5, 60)
            - d256(4)  * arctan_recip(239, 20);

    CHECK(pi > 3.14159265358979323846264338327950288_d256);
    CHECK(pi < 3.14159265358979323846264338327950289_d256);
  }

  // ---- e via 1/n! Taylor with d256 ----
  {
    d256 e(1);
    d256 fact(1);
    for (int n = 1; n <= 40; ++n)
    {
      fact = fact * d256(n);
      e = e + d256(1) / fact;
    }
    CHECK(e > 2.71828182845904523536028747135266249_d256);
    CHECK(e < 2.71828182845904523536028747135266250_d256);
  }
}

//////////////////////////////////////////////////////////////////////////////
// boundaries
//////////////////////////////////////////////////////////////////////////////
void test_boundaries()
{
  SECTION("boundaries");

  // mmax exact
  {
    d32 a(direct, d32::mmax, 0);
    CHECK_EQ(a.sig(), 2147483647);
    CHECK_EQ(a.exp(), 0);
    CHECK_EQ(a, 2147483647_d32);
  }

  // mmax + 1 rounds
  {
    d32 a(direct, d32::mmax, 0);
    d32 b = a + d32(1);
    CHECK_EQ(b.sig(), 214748365);
    CHECK_EQ(b.exp(), 1);
    CHECK(b > a);
  }

  // mmax via literal
  {
    CHECK_EQ((2147483647_d32).sig(), 2147483647);
    CHECK_EQ((2147483647_d32).exp(), 0);
  }

  // d64 mmax via direct (literal too big for the parser)
  {
    d64 a(direct, d64::mmax, 0);
    CHECK_EQ(a.sig(), 9223372036854775807LL);
    CHECK_EQ(a.exp(), 0);
  }

  // static max/min/eps
  {
    CHECK_EQ(d32::max.sig(), 2147483647);
    CHECK_EQ(d32::max.exp(), 32767);
    CHECK_EQ(d32::min.sig(), -2147483647);
    CHECK_EQ(d32::min.exp(), 32767);
    CHECK(!dpp::isnan(d32::eps));
  }

  // max - max == 0
  {
    CHECK_EQ(d32::max - d32::max, d32(0));
  }

  // max / max == 1
  {
    CHECK_EQ(d32::max / d32::max, d32(1));
    CHECK_EQ(d64::max / d64::max, d64(1));
  }

  // max * 10 overflows -> nan
  {
    CHECK(dpp::isnan(d32::max * d32(10)));
  }

  // eps * 1 preserves
  {
    CHECK_EQ(d32::eps * d32(1), d32::eps);
  }

  // eps / 2 is representable
  {
    CHECK(!dpp::isnan(d32::eps / d32(2)));
  }

  // eps identities
  {
    CHECK(d32(1) + d32::eps > d32(1));
    CHECK_EQ(d32(1) + (d32::eps / 2), 1.000000001_d32);   // round-half-up
  }
}

//////////////////////////////////////////////////////////////////////////////
// precision
//////////////////////////////////////////////////////////////////////////////
void test_precision()
{
  SECTION("precision");

  // d32 fits 10-digit significands
  {
    d32 a(999999999);
    CHECK_EQ(a.sig(), 999999999);
    CHECK_EQ(a.exp(), 0);

    d32 b(2147483647LL);
    CHECK_EQ(b.sig(), 2147483647);
    CHECK_EQ(b.exp(), 0);

    d32 c(2147483648LL);
    CHECK_EQ(c.sig(), 214748365);
    CHECK_EQ(c.exp(), 1);
  }

  // d64 fits 19-digit significands
  {
    d64 a(1234567890123456789LL);
    CHECK_EQ(a.sig(), 1234567890123456789LL);
    CHECK_EQ(a.exp(), 0);
  }

  // 0.1 + 0.2 across types
  {
    CHECK_EQ(.1_d32  + .2_d32,  .3_d32);
    CHECK_EQ(.1_d64  + .2_d64,  .3_d64);
    CHECK_EQ(.1_d128 + .2_d128, .3_d128);
    CHECK_EQ(.1_d256 + .2_d256, .3_d256);
  }

  // (1/n) * n is within one ULP of 1; the residual is the rounding
  // error of 1/n, which may be 0 or ±1 ULP depending on n
  {
    CHECK_EQ(d32(1) - d32(1) / d32(3) * d32(3),  .000000001_d32);
    CHECK_EQ(d64(1) - d64(1) / d64(3) * d64(3),  d64(0));
    CHECK_EQ(d128(1) - d128(1) / d128(3) * d128(3),
             .00000000000000000000000000000000000001_d128);

    // residual bounded by 1 ULP (1e-37 is ~10 ULP for d128)
    d128 const ulp = .0000000000000000000000000000000000001_d128;
    for (int n : {6, 7, 9, 11, 13, 17, 19, 23, 29, 31, 37})
    {
      d128 r = d128(1) - d128(1) / d128(n) * d128(n);
      CHECK(dpp::abs(r) <= ulp);
    }

    CHECK_EQ(d32(1) - (d32(1) / d32(3) + d32(1) / d32(3) + d32(1) / d32(3)),
             .000000001_d32);
    CHECK_EQ(d64(1) - (d64(1) / d64(3) + d64(1) / d64(3) + d64(1) / d64(3)),
             d64(0));
  }

  // powers of 10 are exact; compare by value, not by representation
  // (the library does not normalize 10^k to sig = 1, exp = k)
  {
    for (int k = 0; k <= 18; ++k)
    {
      int64_t expected = 1;
      for (int i = 0; i < k; ++i) expected *= 10;

      d64 p(1);
      for (int i = 0; i < k; ++i) p *= d64(10);
      CHECK_EQ(p, d64(expected));
    }
  }

  // negative powers of 10: p * 10^k == 1
  {
    for (int k = 1; k <= 15; ++k)
    {
      int64_t scale = 1;
      for (int i = 0; i < k; ++i) scale *= 10;

      d64 p(1);
      for (int i = 0; i < k; ++i) p /= d64(10);
      CHECK_EQ(p * d64(scale), d64(1));
    }
  }

  // double-then-halve is identity
  {
    for (int k = 0; k < 20; ++k)
    {
      d64 a(1);
      for (int i = 0; i < k; ++i) a *= d64(2);
      for (int i = 0; i < k; ++i) a /= d64(2);
      CHECK_EQ(a, d64(1));
    }
  }

  // catastrophic cancellation at the significand boundary
  {
    CHECK_EQ(d64(1) + .00000000000000000001_d64, d64(1));
    CHECK(d128(1) + .00000000000000000001_d128 > d128(1));
  }

  // adding 1 to 10^18 in d64 (which has 19 digits) is exact
  {
    d64 big(1);
    for (int i = 0; i < 18; ++i) big *= d64(10);
    CHECK_EQ(big + d64(1), d64(1000000000000000001LL));
  }
}

//////////////////////////////////////////////////////////////////////////////
// advanced
//////////////////////////////////////////////////////////////////////////////
void test_advanced()
{
  SECTION("advanced");

  // self-aliasing assignment
  {
    d64 a(7);   a += a;  CHECK_EQ(a, d64(14));
    d64 b(7);   b *= b;  CHECK_EQ(b, d64(49));
    d64 c(100); c -= c;  CHECK_EQ(c, d64(0));
    d64 d(100); d /= d;  CHECK_EQ(d, d64(1));
  }

  // negations
  {
    d64 a(123);
    CHECK_EQ(-a, d64(-123));
    CHECK_EQ(-(-a), a);
    CHECK_EQ(a + (-a), d64(0));
    CHECK_EQ(a - a, d64(0));
  }

  // cross-type equalities
  {
    CHECK(d16(100) == d32(100));
    CHECK(d32(100) == d64(100));
    CHECK(d64(100) == d128(100));
    CHECK(d128(100) == d256(100));
    CHECK(d16(-100) == d256(-100));
  }

  // orderings across types
  {
    CHECK(d16(50) < d32(100));
    CHECK(d32(50) < d64(100));
    CHECK(d64(50) < d128(100));
    CHECK(d256(100) > d16(50));
  }

  // sum of powers of 2 is 2^(n+1) - 1
  {
    for (int n : {10, 20, 30, 40, 50, 60})
    {
      d64 s(0), p(1);
      for (int i = 0; i <= n; ++i) { s += p; p *= d64(2); }
      CHECK_EQ(s, p - d64(1));
    }
  }

  // sum of powers of 3 is (3^(n+1) - 1) / 2
  {
    for (int n : {5, 10, 15, 19})
    {
      d64 s(0), p(1);
      for (int i = 0; i <= n; ++i) { s += p; p *= d64(3); }
      CHECK_EQ(s, (p - d64(1)) / d64(2));
    }
  }

  // triangular numbers via closed form
  {
    for (int n : {10, 100, 1000, 10000})
    {
      d64 s(0);
      for (int i = 1; i <= n; ++i) s += d64(i);
      CHECK_EQ(s, d64(n) * d64(n + 1) / d64(2));
    }
  }

  // commutativity
  {
    d64 a(12345), b(67890);
    CHECK_EQ(a + b, b + a);
    CHECK_EQ(a * b, b * a);
  }

  // associativity of addition for exact cases
  {
    d64 a(1), b(2), c(3);
    CHECK_EQ((a + b) + c, a + (b + c));
  }

  // distributivity for exact cases
  {
    d64 a(7), b(11), c(13);
    CHECK_EQ(a * (b + c), a * b + a * c);
  }

  // transitivity of ordering
  {
    d64 a(1), b(2), c(3);
    CHECK(a < b);
    CHECK(b < c);
    CHECK(a < c);
  }

  // direct construction with different scales compare equal
  {
    d64 a(direct, 100, -2);   // = 1
    d64 b(direct, 1000, -3);  // = 1
    d64 c(direct, 1, 0);      // = 1
    CHECK_EQ(a, b);
    CHECK_EQ(b, c);
    CHECK_EQ(a, c);
  }

  // hash normalizes trailing zeros
  {
    std::hash<d64> h;
    CHECK_EQ(h(d64(direct, 100, -2)),  h(d64(direct, 1, 0)));
    CHECK_EQ(h(d64(direct, 1000, -3)), h(d64(direct, 1, 0)));
    CHECK_EQ(h(d64(direct, -100, -2)), h(d64(direct, -1, 0)));
    CHECK_EQ(h(d64(direct, 12300, -2)), h(d64(123)));
  }

  // hash ignores exponent of zero
  {
    std::hash<d64> h;
    CHECK_EQ(h(d64(0)), h(d64(direct, 0, 0)));
    CHECK_EQ(h(d64(0)), h(d64(direct, 0, 100)));
  }
}

//////////////////////////////////////////////////////////////////////////////
// additional
//////////////////////////////////////////////////////////////////////////////
void test_additional()
{
  SECTION("additional");

  // ---- to_decimal edge cases ----
  {
    CHECK(dpp::isnan(dpp::to_decimal<d32>("")));
    CHECK(dpp::isnan(dpp::to_decimal<d32>("+")));
    CHECK(dpp::isnan(dpp::to_decimal<d32>("-")));
    CHECK(dpp::isnan(dpp::to_decimal<d32>(".")));
    CHECK(dpp::isnan(dpp::to_decimal<d32>("abc")));

    CHECK_EQ(dpp::to_decimal<d32>("0"),        d32(0));
    CHECK_EQ(dpp::to_decimal<d32>("00"),       d32(0));
    CHECK_EQ(dpp::to_decimal<d32>("+0"),       d32(0));
    CHECK_EQ(dpp::to_decimal<d32>("-0"),       d32(0));
    CHECK_EQ(dpp::to_decimal<d32>("0000123"),  d32(123));

    CHECK_EQ(dpp::to_decimal<d32>("+123"),      d32(123));
    CHECK_EQ(dpp::to_decimal<d32>(".5"),        d32(0.5));
    CHECK_EQ(dpp::to_decimal<d32>("0.5"),       d32(0.5));
    CHECK_EQ(dpp::to_decimal<d32>("5."),        d32(5));
    CHECK_EQ(dpp::to_decimal<d32>("5.0"),       d32(5));
    CHECK_EQ(dpp::to_decimal<d32>("1234.5678"), d32(1234.5678));

    // parsing stops at the first non-numeric character
    CHECK_EQ(dpp::to_decimal<d32>("123abc"), d32(123));
    CHECK_EQ(dpp::to_decimal<d32>("123 "),   d32(123));

    // max_v<int32_t> is exactly representable
    CHECK_EQ(dpp::to_decimal<d32>("2147483647"),
             d32(direct, 2147483647, 0));
  }

  // ---- to_string round trips ----
  {
    for (d64 v : {d64(0), d64(1), d64(-1), d64(123), d64(-123),
                  d64(0.5), d64(-0.5), d64(1.5), d64(-1.5),
                  d64(100), d64(1000000), d64(0.001), d64(-0.001)})
    {
      CHECK_EQ(dpp::to_decimal<d64>(dpp::to_string(v)), v);
    }
  }

  // ---- negative numbers ----
  {
    CHECK_EQ(-d32(0), d32(0));
    CHECK_EQ(d32(-0.0), d32(0));

    CHECK_EQ(d64(0)  - d64(5), d64(-5));
    CHECK_EQ(d64(-3) - d64(5), d64(-8));
    CHECK_EQ(d64(-3) * d64(5), d64(-15));
    CHECK_EQ(d64(-3) / d64(5), d64(-0.6));

    CHECK_EQ(dpp::abs(d32::min), d32::max);
    CHECK_EQ(-d32::max,          d32::min);

    CHECK_EQ(dpp::abs(-d64(1.5)), d64(1.5));
    CHECK_EQ(dpp::abs(d64(-1.5)), d64(1.5));
  }

  // ---- rounding ties (half away from zero) ----
  {
    CHECK_EQ(dpp::round(d32( 0.5)), d32( 1));
    CHECK_EQ(dpp::round(d32( 1.5)), d32( 2));
    CHECK_EQ(dpp::round(d32( 2.5)), d32( 3));
    CHECK_EQ(dpp::round(d32(-0.5)), d32(-1));
    CHECK_EQ(dpp::round(d32(-1.5)), d32(-2));
    CHECK_EQ(dpp::round(d32(-2.5)), d32(-3));

    CHECK_EQ(dpp::round(d32( 1.4)), d32( 1));
    CHECK_EQ(dpp::round(d32( 1.6)), d32( 2));
    CHECK_EQ(dpp::round(d32(-1.4)), d32(-1));
    CHECK_EQ(dpp::round(d32(-1.6)), d32(-2));

    CHECK_EQ(dpp::round(d32( 0)), d32( 0));
    CHECK_EQ(dpp::round(d32( 5)), d32( 5));
    CHECK_EQ(dpp::round(d32(-5)), d32(-5));
  }

  // ---- inv more cases ----
  {
    CHECK_EQ(dpp::inv(d32(1)),   d32(1));
    CHECK_EQ(dpp::inv(d32(-1)),  d32(-1));
    CHECK_EQ(dpp::inv(d32(10)),  d32(0.1));
    CHECK_EQ(dpp::inv(d32(0.5)), d32(2));

    CHECK_EQ(dpp::inv(dpp::inv(d64(2))),    d64(2));
    CHECK_EQ(dpp::inv(dpp::inv(d64(4))),    d64(4));
    CHECK_EQ(dpp::inv(dpp::inv(d64(0.5))),  d64(0.5));
    CHECK_EQ(dpp::inv(dpp::inv(d64(0.25))), d64(0.25));
  }

  // ---- midpoint more cases ----
  {
    CHECK_EQ(dpp::midpoint(d32(5), d32(5)),     d32(5));
    CHECK_EQ(dpp::midpoint(d32(0), d32(0)),     d32(0));
    CHECK_EQ(dpp::midpoint(d32(-5), d32(5)),    d32(0));
    CHECK_EQ(dpp::midpoint(d32(3), d32(7)),     d32(5));
    CHECK_EQ(dpp::midpoint(d32(100), d32(200)), d32(150));

    CHECK(dpp::isnan(dpp::midpoint(d32(dpp::nan), d32(5))));
    CHECK(dpp::isnan(dpp::midpoint(d32(5), d32(dpp::nan))));
  }

  // ---- overflow propagation ----
  {
    CHECK(dpp::isnan(d32::max + d32::max));
    CHECK(dpp::isnan(d32::max * d32::max));
    CHECK(dpp::isnan(d32::max * d32(10)));
    CHECK(dpp::isnan(d32::max / d32(0)));
  }

  // ---- boundary exponents ----
  {
    d32 const top(direct, 1, 32767);
    CHECK(!dpp::isnan(top));
    CHECK(static_cast<bool>(top));

    d32 const bot(direct, 1, -32767);
    CHECK(!dpp::isnan(bot));
    CHECK(static_cast<bool>(bot));

    // NaN is encoded as exp == emin: a direct value with exp == emin
    // is interpreted as NaN regardless of its significand.
    CHECK(dpp::isnan(d32(direct, 1, -32768)));
  }

  // ---- identities for exact cases ----
  {
    for (d64 v : {d64(0), d64(1), d64(-1), d64(5), d64(-5), d64(123)})
    {
      CHECK_EQ(v + d64(0), v);
      CHECK_EQ(v - d64(0), v);
      CHECK_EQ(v * d64(1), v);
      CHECK_EQ(v / d64(1), v);
      CHECK_EQ(v * d64(0), d64(0));
      CHECK_EQ(v - v,      d64(0));
      CHECK_EQ(-(-v),      v);
    }
  }

  // ---- algebraic identities for exact cases ----
  {
    d64 const a(7), b(11), c(13);
    CHECK_EQ(a + b, b + a);
    CHECK_EQ(a * b, b * a);
    CHECK_EQ((a + b) + c, a + (b + c));
    CHECK_EQ(a * (b + c), a * b + a * c);
    CHECK_EQ(a - b, -(b - a));
  }

  // ---- construction from exactly-representable binary fractions ----
  {
    CHECK_EQ(d64(0.5), d64(direct, 5, -1));
    CHECK_EQ(d64(0.25), d64(direct, 25, -2));

    // 1/2^k is exactly representable for k up to d64's precision
    CHECK_EQ(d64(0.125)   * d64(8),  d64(1));
    CHECK_EQ(d64(0.0625)  * d64(16), d64(1));
    CHECK_EQ(d64(0.03125) * d64(32), d64(1));

    for (double v : {0.5, 0.25, 0.125, 0.0625, 1.5, 2.5, 100.0,
                     -0.5, -1.5, -0.125, -2.5})
      CHECK_EQ(static_cast<double>(d64(v)), v);
  }

  // ---- bool conversion edge cases ----
  {
    CHECK(!static_cast<bool>(d64(0)));
    CHECK(!static_cast<bool>(d64(direct, 0, 100)));
    CHECK(!static_cast<bool>(d64(direct, 0, -100)));
    CHECK( static_cast<bool>(d64(1)));
    CHECK( static_cast<bool>(d64(-1)));
    CHECK( static_cast<bool>(d64(direct, 1, 100)));
    CHECK( static_cast<bool>(d64(direct, 1, -100)));
  }

  // ---- comparisons across very different direct scales ----
  {
    CHECK(d64(direct, 1,   0) < d64(direct, 2,   0));
    CHECK(d64(direct, 1,   0) < d64(direct, 1,   1));
    CHECK(d64(direct, 1,   1) < d64(direct, 1,   2));
    CHECK(d64(direct, 9,   0) < d64(direct, 1,   1));
    CHECK(d64(direct, 1,  20) > d64(direct, 1, -20));
    CHECK(d64(direct, 1, -20) < d64(direct, 1,  20));

    CHECK_EQ(d64(direct, 100,  -2), d64(direct, 1,   0));
    CHECK_EQ(d64(direct, 100,  -2), d64(direct, 1000, -3));
    CHECK_EQ(d64(direct, 1,    20), d64(direct, 100, 18));
  }

  // ---- trailing-zero normalization in comparisons ----
  {
    CHECK_EQ(d64(direct, 100000, 0), d64(direct, 1, 5));
    CHECK_EQ(d64(direct, 12300,  0), d64(direct, 123, 2));
    CHECK_EQ(d64(direct, -100,   -2), d64(direct, -1, 0));
  }
}

//////////////////////////////////////////////////////////////////////////////
// significand tie
//////////////////////////////////////////////////////////////////////////////
void test_significand_tie()
{
  SECTION("significand tie");

  // mmax + 0 — no rounding, result is exactly mmax
  CHECK_EQ(d32(direct, d32::mmax, 0) + d32(0), d32(direct, 2147483647, 0));

  // mmax + 1 = 2147483648 -> 214748364.8, rounds to 214748365
  CHECK_EQ(d32(direct, d32::mmax, 0) + d32(1), d32(direct, 214748365, 1));

  // mmax + 5 = 2147483652 -> 214748365.2, rounds down
  CHECK_EQ(d32(direct, d32::mmax, 0) + d32(5), d32(direct, 214748365, 1));

  // mmax + 8 = 2147483655 -> 214748365.5, tie between 214748365 (odd)
  // and 214748366 (even); half-up and half-even both pick 214748366
  CHECK_EQ(d32(direct, d32::mmax, 0) + d32(8), d32(direct, 214748366, 1));

  // mmax + 13 = 2147483660 -> 214748366.0, exact
  CHECK_EQ(d32(direct, d32::mmax, 0) + d32(13), d32(direct, 214748366, 1));

  // mmax + 17 = 2147483664 -> 214748366.4, rounds down
  CHECK_EQ(d32(direct, d32::mmax, 0) + d32(17), d32(direct, 214748366, 1));

  // mmax + 18 = 2147483665 -> 214748366.5, tie between 214748366 (even)
  // and 214748367 (odd). This is the tie where half-up and half-even
  // disagree. dpp's half-up rule picks 214748367.
  {
    d32 t = d32(direct, d32::mmax, 0) + d32(18);
    CHECK_EQ(t.sig(), 214748367);
    CHECK_EQ(t.exp(), 1);
    CHECK_EQ(t, d32(direct, 214748367, 1));
  }

  // mmax + 19 = 2147483666 -> 214748366.6, rounds up
  CHECK_EQ(d32(direct, d32::mmax, 0) + d32(19), d32(direct, 214748367, 1));

  // symmetric negative side: mmin - k
  //   mmin = -2147483647

  // mmin - 5 = -2147483652 -> -214748365.2, rounds toward zero in magnitude
  CHECK_EQ(d32(direct, d32::mmin, 0) - d32(5), d32(direct, -214748365, 1));

  // mmin - 8 = -2147483655 -> -214748365.5, tie, half-away-from-zero
  // picks -214748366 (also even)
  CHECK_EQ(d32(direct, d32::mmin, 0) - d32(8), d32(direct, -214748366, 1));

  // mmin - 18 = -2147483665 -> -214748366.5, tie between -214748366 (even)
  // and -214748367 (odd). Half-away-from-zero picks -214748367.
  {
    d32 t = d32(direct, d32::mmin, 0) - d32(18);
    CHECK_EQ(t.sig(), -214748367);
    CHECK_EQ(t.exp(), 1);
    CHECK_EQ(t, d32(direct, -214748367, 1));
  }
}

//////////////////////////////////////////////////////////////////////////////
// type traits and constants
//////////////////////////////////////////////////////////////////////////////
void test_type_traits()
{
  SECTION("type traits");

  // underlying types of the aliases
  static_assert(std::is_same_v<dpp::d16,
    dpp::dpp<std::int16_t, std::int8_t>>);
  static_assert(std::is_same_v<dpp::d24,
    dpp::dpp<intt::intt<std::uint8_t, 3>, std::int8_t>>);
  static_assert(std::is_same_v<dpp::d32,
    dpp::dpp<std::int32_t, std::int16_t>>);
  static_assert(std::is_same_v<dpp::d48,
    dpp::dpp<intt::intt<std::uint16_t, 3>, std::int16_t>>);
  static_assert(std::is_same_v<dpp::d64,
    dpp::dpp<std::int64_t, std::int16_t>>);
  static_assert(std::is_same_v<dpp::d96,
    dpp::dpp<intt::intt<std::uint32_t, 3>, std::int16_t>>);
  static_assert(std::is_same_v<dpp::d128,
    dpp::dpp<intt::intt<std::uint64_t, 2>, std::int32_t>>);
  static_assert(std::is_same_v<dpp::d256,
    dpp::dpp<intt::intt<std::uint64_t, 4>, std::int32_t>>);
  static_assert(std::is_same_v<dpp::d512,
    dpp::dpp<intt::intt<std::uint64_t, 8>, std::int32_t>>);
  static_assert(std::is_same_v<dpp::d1024,
    dpp::dpp<intt::intt<std::uint64_t, 16>, std::int32_t>>);

  // member typedefs
  static_assert(std::is_same_v<dpp::d16::sig_t, std::int16_t>);
  static_assert(std::is_same_v<dpp::d32::sig_t, std::int32_t>);
  static_assert(std::is_same_v<dpp::d64::sig_t, std::int64_t>);
  static_assert(std::is_same_v<dpp::d32::exp_t, std::int16_t>);
  static_assert(std::is_same_v<dpp::d16::exp_t, std::int8_t>);

  // significand limits
  static_assert(dpp::d16::mmax == 32767);
  static_assert(dpp::d16::mmin == -32767);
  static_assert(dpp::d32::mmax == 2147483647);
  static_assert(dpp::d32::mmin == -2147483647);
  static_assert(dpp::d64::mmax == 9223372036854775807LL);
  static_assert(dpp::d64::mmin == -9223372036854775807LL);

  // exponent limits
  static_assert(dpp::d16::emax == 127);
  static_assert(dpp::d16::emin == -128);
  static_assert(dpp::d24::emax == 127);
  static_assert(dpp::d24::emin == -128);
  static_assert(dpp::d32::emax == 32767);
  static_assert(dpp::d32::emin == -32768);
  static_assert(dpp::d64::emax == 32767);
  static_assert(dpp::d64::emin == -32768);
  static_assert(dpp::d128::emax == 2147483647);
  static_assert(dpp::d128::emin == -2147483648);

  // decimal positions of the special values
  static_assert(dpp::d16::eps.sig() == 1);
  static_assert(dpp::d16::eps.exp() == -4);
  static_assert(dpp::d24::eps.exp() == -6);
  static_assert(dpp::d32::eps.sig() == 1);
  static_assert(dpp::d32::eps.exp() == -9);
  static_assert(dpp::d48::eps.exp() == -14);
  static_assert(dpp::d64::eps.exp() == -18);
  static_assert(dpp::d96::eps.exp() == -28);
  static_assert(dpp::d128::eps.exp() == -38);
  static_assert(dpp::d16::max.sig() == 32767);
  static_assert(dpp::d16::max.exp() == 127);
  static_assert(dpp::d32::max.sig() == 2147483647);
  static_assert(dpp::d32::max.exp() == 32767);
  static_assert(dpp::d32::min.sig() == -2147483647);
  static_assert(dpp::d32::min.exp() == 32767);

  // NaN detection is a pure exponent test
  static_assert(!dpp::isnan(dpp::d32::max));
  static_assert(!dpp::isnan(dpp::d32::min));
  static_assert(!dpp::isnan(dpp::d32::eps));
  static_assert(dpp::isnan(dpp::d16(dpp::nan)));
  static_assert(dpp::isnan(dpp::d32(dpp::nan)));
  static_assert(dpp::isnan(dpp::d128(dpp::nan)));
}

//////////////////////////////////////////////////////////////////////////////
// small types (d16, d24, d48)
//////////////////////////////////////////////////////////////////////////////
void test_small_types()
{
  SECTION("small types");

  // ---- d16: 5-digit significand, int8 exponent ----
  {
    d16 a(1234);
    CHECK_EQ(a.sig(), 1234);
    CHECK_EQ(a.exp(), 0);

    // values beyond the significand are rounded half-up
    d16 b(123456);
    CHECK_EQ(b.sig(), 12346);
    CHECK_EQ(b.exp(), 1);

    CHECK_EQ(d16(32767), d16(direct, 32767, 0));
    CHECK_EQ(d16(32768), d16(direct, 3277, 1));
    CHECK_EQ(d16(32768), d16(32770));
    CHECK_EQ(d16(-32768), d16(direct, -3277, 1));
    CHECK_EQ(d16(-32768), d16(-32770));

    CHECK_EQ(d16(0.5), d16(direct, 5, -1));
    CHECK_EQ(d16(1) / d16(4), d16(0.25));
    CHECK_EQ(d16(1) / d16(3), dpp::to_decimal<d16>("0.3333"));
    CHECK_EQ(d16(0.1) + d16(0.2), d16(0.3));

    CHECK_EQ(d16(100) * d16(100), d16(10000));
    CHECK_EQ(d16(200) * d16(200), d16(40000));
    CHECK_EQ(d16(300) * d16(300), d16(direct, 9, 4));

    d16 c(7);
    c += d16(8);   CHECK_EQ(c, d16(15));
    c *= d16(6);   CHECK_EQ(c, d16(90));
    c -= d16(50);  CHECK_EQ(c, d16(40));
    c /= d16(8);   CHECK_EQ(c, d16(5));

    // the exponent is int8: emax = 127, emin = -128 (NaN)
    CHECK(!dpp::isnan(d16(direct, 1, 127)));
    CHECK(!dpp::isnan(d16(direct, 1, -127)));
    CHECK(dpp::isnan(d16(direct, 1, -128)));
    CHECK(dpp::isnan(d16(direct, 0, -128)));

    // overflow at the top exponent
    CHECK(dpp::isnan(d16::max * d16(10)));
    CHECK(dpp::isnan(d16::max + d16::max));
    CHECK_EQ(d16::max + d16(1), d16::max);      // 1 is absorbed

    // underflow to zero
    CHECK_EQ(d16(direct, 1, 127) * d16(direct, 1, -1), d16(direct, 1, 126));
    CHECK_EQ(d16(direct, 1, -64) * d16(direct, 1, -64), d16(0));

    CHECK_EQ(dpp::abs(d16::min), d16::max);
    CHECK_EQ(-d16::max, d16::min);
    CHECK(d16(1) + d16::eps > d16(1));
  }

  // ---- d24: 24-bit intt significand, int8 exponent ----
  {
    CHECK_EQ(d24(1000), d24(direct, 1000, 0));
    CHECK_EQ(d24(8388607), d24(direct, 8388607, 0));
    CHECK_EQ(d24(8388608), d24(direct, 838861, 1));

    CHECK_EQ(dpp::to_string(dpp::d24(direct, d24::mmax, 0)),
      std::string("8388607"));
    CHECK_EQ(dpp::to_string(dpp::d24(8388608)), std::string("8388610"));

    CHECK_EQ(d24(100) * d24(200), d24(20000));
    CHECK_EQ(d24(1) / d24(3), dpp::to_decimal<d24>("0.3333333"));
    CHECK_EQ(d24(0.1) + d24(0.2), d24(0.3));
    CHECK_EQ(dpp::to_string(dpp::d24(direct, 15, -1)), std::string("1.5"));

    CHECK(dpp::isnan(d24(direct, 1, -128)));
    CHECK(!dpp::isnan(d24::max));
    CHECK(d24(1) + d24::eps > d24(1));
    CHECK_EQ(static_cast<int>(d24::eps.exp()), -6);
  }

  // ---- d48: 48-bit intt significand, int16 exponent ----
  {
    CHECK_EQ(dpp::to_string(dpp::d48(direct, d48::mmax, 0)),
      std::string("140737488355327"));
    CHECK_EQ(d48(140737488355327LL), d48(direct, d48::mmax, 0));

    CHECK_EQ(d48(123456) * d48(654321), d48(80779853376LL));
    CHECK_EQ(d48(1) / d48(3), dpp::to_decimal<d48>("0.33333333333333"));
    CHECK_EQ(d48(0.1) + d48(0.2), d48(0.3));

    CHECK_EQ(dpp::to_string(dpp::d48(direct, 12345, -3)),
      std::string("12.345"));
    CHECK(dpp::isnan(d48::max * d48(10)));
    CHECK(d48(1) + d48::eps > d48(1));
    CHECK_EQ(static_cast<int>(d48::eps.exp()), -14);
  }
}

//////////////////////////////////////////////////////////////////////////////
// large types (d96, d512, d1024)
//////////////////////////////////////////////////////////////////////////////
void test_large_types()
{
  SECTION("large types");

  // ---- d96: 96-bit intt significand ----
  {
    CHECK_EQ(dpp::to_string(dpp::d96(direct, d96::mmax, 0)),
      std::string("39614081257132168796771975167"));

    CHECK_EQ(d96(1000000000) * d96(1000000000),
      d96(1000000000000000000LL));
    CHECK_EQ(d96(1) / d96(3),
      dpp::to_decimal<d96>(std::string("0.") + std::string(29, '3')));
    CHECK_EQ(d96(0.1) + d96(0.2), d96(0.3));
    CHECK_EQ(dpp::to_string(dpp::d96(direct, 1, 28)),
      std::string("1") + std::string(28, '0'));
    CHECK_EQ(static_cast<int>(d96::eps.exp()), -28);
    CHECK(d96(1) + d96::eps > d96(1));
    CHECK(dpp::isnan(d96::max * d96(10)));
  }

  // ---- d512 ----
  {
    CHECK_EQ(dpp::to_string(dpp::d512(98765)), std::string("98765"));
    CHECK_EQ(dpp::to_string(dpp::d512(direct, 15, -1)), std::string("1.5"));
  }

  // ---- d1024 ----
  {
    CHECK_EQ(dpp::to_string(dpp::d1024(12345)), std::string("12345"));
  }
}

//////////////////////////////////////////////////////////////////////////////
// cross-type promotions
//////////////////////////////////////////////////////////////////////////////
void test_cross_type_promotions()
{
  SECTION("cross-type promotions");

  {
    auto r = d16(1) + d32(2);
    static_assert(std::is_same_v<decltype(r), d32>);
    CHECK_EQ(r, d32(3));
  }
  {
    auto r = d32(1) + d16(2);
    static_assert(std::is_same_v<decltype(r), d32>);
    CHECK_EQ(r, d32(3));
  }
  {
    auto r = d32(1) * d64(2);
    static_assert(std::is_same_v<decltype(r), d64>);
    CHECK_EQ(r, d64(2));
  }
  {
    auto r = d64(9) / d32(3);
    static_assert(std::is_same_v<decltype(r), d64>);
    CHECK_EQ(r, d64(3));
  }
  {
    auto r = d64(1) + d128(2);
    static_assert(std::is_same_v<decltype(r), d128>);
    CHECK_EQ(r, d128(3));
  }
  {
    auto r = d128(1) * d256(2);
    static_assert(std::is_same_v<decltype(r), d256>);
    CHECK_EQ(r, d256(2));
  }
  {
    auto r = d24(1) + d48(2);
    static_assert(std::is_same_v<decltype(r), d48>);
    CHECK_EQ(r, d48(3));
  }
  {
    auto r = d48(1) + d96(2);
    static_assert(std::is_same_v<decltype(r), d96>);
    CHECK_EQ(r, d96(3));
  }
  {
    auto r = d16(5) - d64(2);
    static_assert(std::is_same_v<decltype(r), d64>);
    CHECK_EQ(r, d64(3));
  }

  // comparisons promote as well
  CHECK(d16(50) < d96(100));
  CHECK(d48(100) > d24(50));
  CHECK(d256(100) == d16(100));

  // chains keep promoting to the widest operand
  CHECK_EQ(d16(1) + d32(2) + d64(3), d64(6));
  CHECK_EQ(d256(1) + d16(2) + d128(3), d256(6));

  // compound assignment absorbs the promoted result
  d32 a(1);
  a += d64(2);
  CHECK_EQ(a, d32(3));
  d64 b(2);
  b *= d16(3);
  CHECK_EQ(b, d64(6));

  // NaN propagates through promotions
  CHECK(dpp::isnan(d16(1) + d32(dpp::nan)));
  CHECK(dpp::isnan(d128(1) * d16(dpp::nan)));
}

//////////////////////////////////////////////////////////////////////////////
// native type interop
//////////////////////////////////////////////////////////////////////////////
void test_native_interop()
{
  SECTION("native type interop");

  // integers
  CHECK_EQ(d32(5) + std::int8_t(3), d32(8));
  CHECK_EQ(std::int16_t(3) + d32(5), d32(8));
  CHECK_EQ(d64(5) + std::int64_t(3), d64(8));
  CHECK_EQ(std::uint32_t(3) + d32(5), d32(8));
  CHECK_EQ(d32(5) + 3u, d32(8));
  CHECK_EQ(d32(5) - 10u, d32(-5));
  CHECK_EQ(d32(5) * std::uint64_t(2), d32(10));
  CHECK_EQ(d64(10) / std::uint16_t(4), d64(2.5));
  CHECK_EQ(d32(5) + char(3), d32(8));
  CHECK_EQ(d32(true) + d32(1), d32(2));
  CHECK(d32(1) == true);
  CHECK(d32(false) == d32(0));
  CHECK(d32(0) == false);

  // floating point
  CHECK_EQ(d32(1.5f), d32(1.5));
  CHECK_EQ(d64(2.25f) * d64(2), d64(4.5));
  CHECK_EQ(d32(2) * 0.5, d32(1));
  CHECK_EQ(0.5 * d32(2), d32(1));
  CHECK_EQ(d32(10) / 4.0, d32(2.5));
  CHECK_EQ(10 / d32(4), d32(2.5));
  CHECK_EQ(d64(1.5L) * d64(2), d64(3));
  CHECK_EQ(d128(1) / 4, d128(0.25));
  CHECK_EQ(d32(0.1f), d32(0.1));  // float 0.1 rounds to the same d32
  CHECK_EQ(static_cast<float>(d32(0.1f)), 0.1f);

  // comparisons with natives
  CHECK(d32(1.5) > 1);
  CHECK(d32(1.5) > 1.0);
  CHECK(2 > d32(1.5));
  CHECK(1.0 < d32(1.5));
  CHECK(d64(1) == 1.0);
  CHECK(d64(1) != 1.5);
  CHECK(d16(1) == std::int64_t(1));
  CHECK(d32(-1) < 0);

  // compound assignment with natives
  d32 a(10);
  a += 5;     CHECK_EQ(a, d32(15));
  a -= 2.5;   CHECK_EQ(a, d32(12.5));
  a *= 4;     CHECK_EQ(a, d32(50));
  a /= 8u;    CHECK_EQ(a, d32(6.25));
  d64 b(1);
  b += 0.5;   CHECK_EQ(b, d64(1.5));
  b *= 2;     CHECK_EQ(b, d64(3));
  b /= 3;     CHECK_EQ(b, d64(1));

  // explicit conversions
  CHECK_EQ(static_cast<std::int64_t>(d64(1234567890123LL)),
    1234567890123LL);
  CHECK_EQ(static_cast<std::int32_t>(d32(-42.9)), -42);
  CHECK_EQ(static_cast<std::int32_t>(d32(-1.9)), -1);
  CHECK_EQ(static_cast<double>(d64(0.125)), 0.125);
  CHECK_EQ(static_cast<long double>(d64(1.5)), 1.5L);
  CHECK_EQ(static_cast<float>(d32(2.5)), 2.5f);
  CHECK(std::isnan(static_cast<double>(d64(dpp::nan))));
}

//////////////////////////////////////////////////////////////////////////////
// compile-time evaluation
//////////////////////////////////////////////////////////////////////////////
void test_constexpr()
{
  SECTION("constexpr");

  // arithmetic
  static_assert(d32(1) + d32(2) == d32(3));
  static_assert(d32(1) - d32(1) == d32(0));
  static_assert(d32(3) * d32(4) == d32(12));
  static_assert(d32(12) / d32(4) == d32(3));
  static_assert(dpp::isnan(d32(1) / d32(0)));
  static_assert(-(-d32(5)) == d32(5));
  static_assert(d32(1) + d32(-1) == d32(0));

  // utilities
  static_assert(dpp::abs(d32(-5)) == d32(5));
  static_assert(dpp::trunc(d32(1.5)) == d32(1));
  static_assert(dpp::trunc(d32(-1.5)) == d32(-1));
  static_assert(dpp::floor(d32(-1.5)) == d32(-2));
  static_assert(dpp::ceil(d32(1.5)) == d32(2));
  static_assert(dpp::round(d32(1.5)) == d32(2));
  static_assert(dpp::round(d32(-2.5)) == d32(-3));
  static_assert(dpp::inv(4_d32) == 0.25_d32);
  static_assert(dpp::midpoint(d32(0), d32(10)) == d32(5));
  static_assert(dpp::fma(d32(2), d32(3), d32(4)) == d32(10));

  // parsing and literals
  static_assert(dpp::to_decimal<d32>("123") == d32(123));
  static_assert(dpp::to_decimal<d32>("-1.5") == d32(-1.5));
  static_assert(.1_d32 + .2_d32 == .3_d32);
  static_assert(.1_d32 + .2_d16 == .3_d32);
  static_assert(dpp::isnan(dpp::to_decimal<d32>("abc")));

  // accessors and special values
  static_assert(d32(direct, 42, -1).sig() == 42);
  static_assert(d32(direct, 42, -1).exp() == -1);
  static_assert(!static_cast<bool>(d32(0)));
  static_assert(static_cast<bool>(d32(1)));
  static_assert(dpp::isnan(d32(dpp::nan)));
  static_assert(!dpp::isnan(d32(1)));

  // saturation behaviour at the top exponent
  static_assert(dpp::isnan(d32::max + d32::max));
  static_assert(dpp::isnan(d32::max * d32(10)));
  static_assert(d32::max / d32::max == d32(1));
  static_assert(d32::max - d32::max == d32(0));

  // cross-type and intt arithmetic in constant expressions
  static_assert(2_d128 * 2_d128 == 4_d128);
  static_assert(d64(1) + d32(2) == d64(3));
  static_assert(dpp::abs(d64(-7)) == d64(7));

  // constexpr variables usable at runtime
  constexpr d32 c1 = d32(2) * d32(21);
  CHECK_EQ(c1, d32(42));
  constexpr auto c2 = dpp::inv(3_d32);
  CHECK_EQ(c2, dpp::to_decimal<d32>("0.333333333"));
  constexpr auto c3 = 12345_d64 + 1_d64;
  CHECK_EQ(c3, d64(12346));
  constexpr auto c4 = dpp::midpoint(d32(1), d32(2));
  CHECK_EQ(c4, d32(1.5));
}

//////////////////////////////////////////////////////////////////////////////
// three-way ordering
//////////////////////////////////////////////////////////////////////////////
void test_ordering()
{
  SECTION("three-way ordering");

  using std::partial_ordering;

  CHECK((d32(1) <=> d32(2)) == partial_ordering::less);
  CHECK((d32(2) <=> d32(1)) == partial_ordering::greater);
  CHECK((d32(1) <=> d32(1)) == partial_ordering::equivalent);

  // equal values with different representations are equivalent
  CHECK((d32(1) <=> d32(direct, 10, -1)) == partial_ordering::equivalent);
  CHECK((d32(direct, 100, -2) <=> d32(direct, 1, 0)) ==
    partial_ordering::equivalent);

  // NaN is unordered with everything (including itself)
  CHECK((d32(dpp::nan) <=> d32(1)) == partial_ordering::unordered);
  CHECK((d32(1) <=> d32(dpp::nan)) == partial_ordering::unordered);
  CHECK((d32(dpp::nan) <=> d32(dpp::nan)) == partial_ordering::unordered);

  CHECK(std::is_lt(d32(1) <=> d32(2)));
  CHECK(std::is_gt(d32(2) <=> d32(1)));
  CHECK(std::is_eq(d32(1) <=> d32(1)));
  CHECK(std::is_neq(d32(dpp::nan) <=> d32(1)));

  // cross-type and native comparisons
  CHECK((d16(1) <=> d32(2)) == partial_ordering::less);
  CHECK((d64(2) <=> d16(1)) == partial_ordering::greater);
  CHECK((d32(1) <=> 2) == partial_ordering::less);
  CHECK((2 <=> d32(1)) == partial_ordering::greater);
  CHECK((d32(1) <=> 1.0) == partial_ordering::equivalent);
}

//////////////////////////////////////////////////////////////////////////////
// rounding and alignment
//////////////////////////////////////////////////////////////////////////////
void test_rounding_and_alignment()
{
  SECTION("rounding and alignment");

  // alignment uses the wider sig2_t registers when scaling up
  CHECK_EQ(d32(10000000000LL) + d32(1), d32(10000000001LL));
  CHECK_EQ(d32(1e9) + d32(0.5), d32(1000000001));          // half-up
  CHECK_EQ(d32(2147483647) + d32(0.5), d32(2147483650));   // half-up
  CHECK_EQ(d64(2147483647) + d64(0.5), d64(2147483647.5)); // d64 has headroom

  // values that cannot be aligned are absorbed
  CHECK_EQ(d32(1000000000) - d32::eps, d32(1000000000));
  CHECK_EQ(d32(5) + d32(direct, 0, -32767), d32(5));
  CHECK_EQ(d32(5) + d32(direct, 0, 32767), d32(5));

  // midpoint across scales
  CHECK_EQ(dpp::midpoint(d32(1), d32(2)), d32(1.5));
  CHECK_EQ(dpp::midpoint(d32(1), d32(3)), d32(2));
  CHECK_EQ(dpp::midpoint(d32(0.1), d32(0.2)), d32(0.15));
  CHECK_EQ(dpp::midpoint(d32(0), d32(1)), d32(0.5));
  CHECK_EQ(dpp::midpoint(d32(-10), d32(10)), d32(0));
  CHECK_EQ(dpp::midpoint(d32(1), d32(10000000000LL)), d32(5000000000LL));
  CHECK(dpp::isnan(dpp::midpoint(d32(1), d32(dpp::nan))));

  // fma across scales
  CHECK_EQ(dpp::fma(d32(2), d32(3), d32(0.5)), d32(6.5));
  CHECK_EQ(dpp::fma(d32(0.5), d32(0.5), d32(0.25)), d32(0.5));
  CHECK_EQ(dpp::fma(d32(1), d32(2), d32(-3)), d32(-1));
  CHECK(dpp::isnan(dpp::fma(d32(1), d32(dpp::nan), d32(1))));
  CHECK(dpp::isnan(dpp::fma(d32(1), d32(1), d32(dpp::nan))));

  // trunc/floor/ceil/round at negative halves
  CHECK_EQ(dpp::floor(d32(-0.5)), d32(-1));
  CHECK_EQ(dpp::ceil(d32(-0.5)), d32(0));
  CHECK_EQ(dpp::trunc(d32(-0.5)), d32(0));
  CHECK_EQ(dpp::round(d32(0.5)), d32(1));

  // increments across scales
  d32 inc(0.5);
  CHECK_EQ(++inc, d32(1.5));
  d32 dec(1.5);
  CHECK_EQ(--dec, d32(0.5));

  // adding one to the saturated maximum overflows to NaN
  CHECK(dpp::isnan(d32::max + d32(1)));
  CHECK(dpp::isnan(d32::min - d32(1)));
}

//////////////////////////////////////////////////////////////////////////////
// exponent extremes
//////////////////////////////////////////////////////////////////////////////
void test_exponent_extremes()
{
  SECTION("exponent extremes");

  // exponent overflow produces NaN
  d32 p(direct, 1, 32000);
  p *= d32(direct, 1, 700);
  CHECK_EQ(p, d32(direct, 1, 32700));
  p *= d32(direct, 1, 100);
  CHECK(dpp::isnan(p));

  // exponent underflow flushes to zero
  CHECK_EQ(d32(direct, 1, -20000) * d32(direct, 1, -20000), d32(0));
  CHECK_EQ(d64(direct, 1, -20000) * d64(direct, 1, -20000), d64(0));
  CHECK_EQ(d32(direct, 1, -32767) * d32(direct, 1, -1), d32(0));
  CHECK_EQ(d32(direct, 1, 32767) * d32(0), d32(0));

  // dividing by a huge power of ten stays representable
  CHECK_EQ(d32(5) / d32(direct, 1, 32767), d32(direct, 5, -32767));
  CHECK_EQ(d32(direct, 5, -32767) * d32(1), d32(direct, 5, -32767));

  // d16's int8 exponent
  CHECK_EQ(d16(direct, 1, 127) * d16(direct, 1, -1), d16(direct, 1, 126));
  CHECK_EQ(d16(direct, 1, -127) * d16(direct, 1, -1), d16(0));

  // exponents at emin encode NaN regardless of the significand
  CHECK(dpp::isnan(d32(direct, 1, -32768)));
  CHECK(dpp::isnan(d32(direct, 0, -32768)));
  CHECK(dpp::isnan(d32(direct, 2147483647, -32768)));
  CHECK(!dpp::isnan(d32(direct, 1, -32767)));
  CHECK(!dpp::isnan(d32(direct, 1, 32767)));
}

//////////////////////////////////////////////////////////////////////////////
// to_decimal details
//////////////////////////////////////////////////////////////////////////////
void test_to_decimal_more()
{
  SECTION("to_decimal details");

  // integer overflow shifts into the exponent, keeping the top digits
  CHECK_EQ(dpp::to_decimal<d32>("9999999999"), d32(9999999990LL));
  CHECK_EQ(dpp::to_decimal<d32>("1234567890123456789"),
    d32(direct, 123456789, 10));
  CHECK_EQ(dpp::to_decimal<d64>("9223372036854775808"),
    d64(direct, 922337203685477580LL, 1));

  // the full significand can be filled
  CHECK_EQ(dpp::to_decimal<d64>("9223372036854775807"),
    d64(direct, 9223372036854775807LL, 0));
  CHECK_EQ(dpp::to_decimal<d32>("2147483647"), d32(direct, 2147483647, 0));

  // fractional digits accumulate until the significand is full
  CHECK_EQ(dpp::to_decimal<d32>("0.123456789012345"),
    d32(direct, 1234567890, -10));

  // tiny values
  CHECK_EQ(dpp::to_decimal<d32>("0.0000001"), d32(direct, 1, -7));
  CHECK_EQ(dpp::to_decimal<d32>("0.00000000000000000001"),
    d32(direct, 1, -20));

  // leading zeros, signs and redundant characters
  CHECK_EQ(dpp::to_decimal<d32>("+007"), d32(7));
  CHECK_EQ(dpp::to_decimal<d32>("000.5"), d32(0.5));
  CHECK_EQ(dpp::to_decimal<d32>("-.5"), d32(-0.5));
  CHECK_EQ(dpp::to_decimal<d32>("+.5"), d32(0.5));
  CHECK_EQ(dpp::to_decimal<d32>("-0"), d32(0));
  CHECK_EQ(dpp::to_decimal<d32>("0000123"), d32(123));

  // trailing zeros are kept in the representation
  CHECK_EQ(dpp::to_decimal<d32>("1.50"), d32(direct, 150, -2));
  CHECK_EQ(dpp::to_decimal<d32>("1.50"), d32(1.5));

  // string_view and iterator ranges
  CHECK_EQ(dpp::to_decimal<d64>(std::string_view("2.25")), d64(2.25));
  {
    char const s[] = "3.5";
    CHECK_EQ(dpp::to_decimal<d64>(std::begin(s), std::end(s) - 1),
      d64(3.5));
  }
  {
    std::vector<char> const v{'1', '2', '3'};
    CHECK_EQ(dpp::to_decimal<d32>(v.begin(), v.end()), d32(123));
  }
  {
    std::vector<char> const v{'1', '.', '5'};
    CHECK_EQ(dpp::to_decimal<d32>(v.begin(), v.end()), d32(1.5));
  }
  {
    std::vector<char> const v{};
    CHECK(dpp::isnan(dpp::to_decimal<d32>(v.begin(), v.end())));
  }

  // parsing stops at the first non-numeric character
  CHECK_EQ(dpp::to_decimal<d32>("12abc"), d32(12));
  CHECK_EQ(dpp::to_decimal<d32>("1 2"), d32(1));
}

//////////////////////////////////////////////////////////////////////////////
// to_string details
//////////////////////////////////////////////////////////////////////////////
void test_to_string_more()
{
  SECTION("to_string details");

  // positive exponents append zeros
  CHECK_EQ(dpp::to_string(d32(direct, 123, 2)), std::string("12300"));
  CHECK_EQ(dpp::to_string(d32(direct, 1, 9)), std::string("1000000000"));
  CHECK_EQ(dpp::to_string(d64(direct, 1, 18)),
    std::string("1000000000000000000"));
  CHECK_EQ(dpp::to_string(d32(direct, -123, 2)), std::string("-12300"));

  // negative exponents: trailing zeros are slashed first
  CHECK_EQ(dpp::to_string(d64(direct, 12300, -2)), std::string("123"));
  CHECK_EQ(dpp::to_string(d64(direct, -12300, -2)), std::string("-123"));
  CHECK_EQ(dpp::to_string(d32(direct, 150, -2)), std::string("1.5"));
  CHECK_EQ(dpp::to_string(d32(direct, 5, -3)), std::string("0.005"));
  CHECK_EQ(dpp::to_string(d32(direct, -5, -3)), std::string("-0.005"));
  CHECK_EQ(dpp::to_string(d32(direct, 1, -9)), std::string("0.000000001"));

  // zero prints as "0" regardless of exponent or sign
  CHECK_EQ(dpp::to_string(d32(direct, 0, 5)), std::string("0"));
  CHECK_EQ(dpp::to_string(d32(direct, 0, -5)), std::string("0"));
  CHECK_EQ(dpp::to_string(-d32(0)), std::string("0"));

  // intt-backed significands
  CHECK_EQ(dpp::to_string(d24(123)), std::string("123"));
  CHECK_EQ(dpp::to_string(d24(direct, 15, -1)), std::string("1.5"));
  CHECK_EQ(dpp::to_string(d48(direct, 12345, -3)), std::string("12.345"));
  CHECK_EQ(dpp::to_string(d128(direct, 15, -1)), std::string("1.5"));
  CHECK_EQ(dpp::to_string(d256(direct, 12345, -2)), std::string("123.45"));
  CHECK_EQ(dpp::to_string(d96(direct, 1, 28)),
    std::string("1") + std::string(28, '0'));

  // scaled representations slash their trailing zeros
  CHECK_EQ(dpp::to_string(d32(1.5)), std::string("1.5"));
  CHECK_EQ(dpp::to_string(d64(0.125)), std::string("0.125"));
  CHECK_EQ(dpp::to_string(d32(-2.25)), std::string("-2.25"));
}

//////////////////////////////////////////////////////////////////////////////
// stream extraction details
//////////////////////////////////////////////////////////////////////////////
void test_stream_more()
{
  SECTION("stream extraction details");

  {
    std::istringstream is("+5");
    d32 a; is >> a;
    CHECK_EQ(a, d32(5));
    CHECK(!is.fail());
  }
  {
    std::istringstream is(".5");
    d32 a; is >> a;
    CHECK_EQ(a, d32(0.5));
    CHECK(!is.fail());
  }
  {
    std::istringstream is("-.5");
    d32 a; is >> a;
    CHECK_EQ(a, d32(-0.5));
    CHECK(!is.fail());
  }
  {
    std::istringstream is("007");
    d32 a; is >> a;
    CHECK_EQ(a, d32(7));
    CHECK(!is.fail());
  }
  {
    std::istringstream is("1e5");
    d32 a; is >> a;
    CHECK_EQ(a, d32(1));            // parsing stops at 'e'
    CHECK(!is.fail());
  }
  {
    std::istringstream is("123abc");
    d32 a; is >> a;
    CHECK_EQ(a, d32(123));
    CHECK(!is.fail());
  }
  {
    std::istringstream is("1 2 3");
    d32 a, b, c;
    is >> a >> b >> c;
    CHECK_EQ(a, d32(1));
    CHECK_EQ(b, d32(2));
    CHECK_EQ(c, d32(3));
  }
  {
    std::istringstream is("1234567890123456789");
    d64 a; is >> a;
    CHECK_EQ(a, d64(1234567890123456789LL));
    CHECK(!is.fail());
  }
  {
    std::istringstream is("nanabc");
    d32 a; is >> a;
    CHECK(dpp::isnan(a));
    CHECK(!is.fail());

    d32 b; is >> b;                 // the rest is not a number
    CHECK(is.fail());
  }
  {
    std::istringstream is("na");
    d32 a; is >> a;
    CHECK(is.fail());               // incomplete nan
  }
  {
    std::istringstream is("abc");
    d32 a(5);
    is >> a;
    CHECK(is.fail());
    CHECK(dpp::isnan(a));           // target is assigned NaN
  }
  {
    std::istringstream is("");
    d32 a; is >> a;
    CHECK(is.fail());               // sentry fails on an empty stream
  }
  {
    std::istringstream is("3.14159265358979");
    d64 a; is >> a;
    CHECK_EQ(a, dpp::to_decimal<d64>("3.14159265358979"));
  }
  {
    std::istringstream is("1.5");
    d16 a; is >> a;
    CHECK_EQ(a, d16(1.5));
  }
}

//////////////////////////////////////////////////////////////////////////////
// square roots (sqrt.hpp)
//////////////////////////////////////////////////////////////////////////////
void test_sqrt()
{
  SECTION("sqrt");

  // perfect squares are exact across all types
  CHECK_EQ(dpp::sqrt(d16(4)), d16(2));
  CHECK_EQ(dpp::sqrt(d16(9)), d16(3));
  CHECK_EQ(dpp::sqrt(d24(4)), d24(2));
  CHECK_EQ(dpp::sqrt(d24(9)), d24(3));
  CHECK_EQ(dpp::sqrt(d32(4)), d32(2));
  CHECK_EQ(dpp::sqrt(d32(9)), d32(3));
  CHECK_EQ(dpp::sqrt(d48(4)), d48(2));
  CHECK_EQ(dpp::sqrt(d48(9)), d48(3));
  CHECK_EQ(dpp::sqrt(d64(4)), d64(2));
  CHECK_EQ(dpp::sqrt(d64(9)), d64(3));
  CHECK_EQ(dpp::sqrt(d64(1000000)), d64(1000));
  CHECK_EQ(dpp::sqrt(d128(9)), d128(3));
  CHECK_EQ(dpp::sqrt(d32(10000)), d32(100));
  CHECK_EQ(dpp::sqrt(d64(1000000000000000000LL)), d64(1000000000LL));

  // sqrt(0) == 0 for every type (both integral and intt significands);
  // the zero significand is returned before the scaling loop is entered
  CHECK_EQ(dpp::sqrt(d16(0)), d16(0));
  CHECK_EQ(dpp::sqrt(d24(0)), d24(0));
  CHECK_EQ(dpp::sqrt(d32(0)), d32(0));
  CHECK_EQ(dpp::sqrt(d48(0)), d48(0));
  CHECK_EQ(dpp::sqrt(d64(0)), d64(0));
  CHECK_EQ(dpp::sqrt(d96(0)), d96(0));
  CHECK_EQ(dpp::sqrt(d128(0)), d128(0));

  // zero with a non-canonical exponent still yields zero
  CHECK_EQ(dpp::sqrt(d32(direct, 0, -5)), d32(0));
  CHECK_EQ(dpp::sqrt(d64(direct, 0, 10)), d64(0));

  {
    // the result is the canonical zero: significand 0, exponent 0, falsy
    auto const z = dpp::sqrt(d32(0));
    CHECK_EQ(z.sig(), 0);
    CHECK_EQ(z.exp(), 0);
    CHECK(!static_cast<bool>(z));
  }

  // sqrt(0) is constexpr-friendly (it used to loop forever at compile time)
  static_assert(dpp::sqrt(dpp::d16(0)) == dpp::d16(0));
  static_assert(dpp::sqrt(dpp::d32(0)) == dpp::d32(0));
  static_assert(dpp::sqrt(dpp::d64(0)) == dpp::d64(0));

  // sqrt of a negative value is NaN (IEEE-754), for every significand kind
  CHECK(dpp::isnan(dpp::sqrt(d16(-4))));
  CHECK(dpp::isnan(dpp::sqrt(d24(-4))));
  CHECK(dpp::isnan(dpp::sqrt(d32(-4))));
  CHECK(dpp::isnan(dpp::sqrt(d48(-4))));
  CHECK(dpp::isnan(dpp::sqrt(d64(-4))));
  CHECK(dpp::isnan(dpp::sqrt(d96(-4))));
  CHECK(dpp::isnan(dpp::sqrt(d128(-4))));

  // ...also with a fractional magnitude and a negative exponent
  CHECK(dpp::isnan(dpp::sqrt(d32(direct, -4, -2))));  // -0.04
  CHECK(dpp::isnan(dpp::sqrt(d64(direct, -25, -3)))); // -0.025

  // sqrt of NaN is NaN; the guard short-circuits before the significand
  // is ever read
  CHECK(dpp::isnan(dpp::sqrt(d16(dpp::nan))));
  CHECK(dpp::isnan(dpp::sqrt(d24(dpp::nan))));
  CHECK(dpp::isnan(dpp::sqrt(d32(dpp::nan))));
  CHECK(dpp::isnan(dpp::sqrt(d48(dpp::nan))));
  CHECK(dpp::isnan(dpp::sqrt(d64(dpp::nan))));
  CHECK(dpp::isnan(dpp::sqrt(d96(dpp::nan))));
  CHECK(dpp::isnan(dpp::sqrt(d128(dpp::nan))));

  // negative/NaN inputs are rejected at compile time too, and the finite
  // non-negative results are unaffected by the new guards
  static_assert(dpp::isnan(dpp::sqrt(dpp::d32(-4))));
  static_assert(dpp::isnan(dpp::sqrt(dpp::d128(-9))));
  static_assert(dpp::isnan(dpp::sqrt(dpp::d32(dpp::nan))));
  static_assert(dpp::isnan(dpp::sqrt(dpp::d128(dpp::nan))));
  static_assert(dpp::sqrt(dpp::d64(0)) == dpp::d64(0));
  static_assert(dpp::sqrt(dpp::d64(4)) == dpp::d64(2));

  CHECK_EQ(dpp::sqrt(d64(1)), d64(1));

  // sqrt(2) approximations, tighter with more precision
  {
    auto s = dpp::sqrt(d16(2));
    CHECK(s > dpp::to_decimal<d16>("1.41"));
    CHECK(s < dpp::to_decimal<d16>("1.42"));
  }
  {
    auto s = dpp::sqrt(d24(2));
    CHECK(s > dpp::to_decimal<d24>("1.41421"));
    CHECK(s < dpp::to_decimal<d24>("1.41422"));
  }
  {
    auto s = dpp::sqrt(d32(2));
    CHECK(s > dpp::to_decimal<d32>("1.414213561"));
    CHECK(s < dpp::to_decimal<d32>("1.414213563"));
  }
  {
    auto s = dpp::sqrt(d48(2));
    CHECK(s > dpp::to_decimal<d48>("1.414213562373"));
    CHECK(s < dpp::to_decimal<d48>("1.414213562374"));
  }
  {
    auto s = dpp::sqrt(d64(2));
    CHECK(s > dpp::to_decimal<d64>("1.414213562373095048"));
    CHECK(s < dpp::to_decimal<d64>("1.414213562373095050"));
    CHECK(std::abs(double(s) - std::sqrt(2.0)) < 1e-15);
  }
  {
    auto s = dpp::sqrt(d128(2));
    CHECK(s > dpp::to_decimal<d128>(
      "1.4142135623730950488016887242096980785"));
    CHECK(s < dpp::to_decimal<d128>(
      "1.4142135623730950488016887242096980787"));
  }

  // another irrational
  {
    auto s = dpp::sqrt(d64(12345));
    CHECK(s > dpp::to_decimal<d64>("111.10805551354051"));
    CHECK(s < dpp::to_decimal<d64>("111.10805551354052"));
  }

  // identity: 1 / (1 + sqrt(2)) == sqrt(2) - 1, verified against d128
  {
    auto t = d128(1) / (d128(1) + dpp::sqrt(d128(2)));
    CHECK(t > dpp::to_decimal<d128>(
      "0.4142135623730950488016887242096980785"));
    CHECK(t < dpp::to_decimal<d128>(
      "0.4142135623730950488016887242096980786"));
  }
}

//////////////////////////////////////////////////////////////////////////////
// elementary functions computed with dpp arithmetic
// (exp, log, sin, cos, tan, atan, sinh, cosh, tanh, pow -- built only on
// dpp arithmetic and dpp::sqrt; the checks pin results to 30 digit bounds)
//////////////////////////////////////////////////////////////////////////////
namespace
{

// exp(x): halve the argument until it is at most 1/2, sum the Taylor
// series, then square back up: exp(x) = exp(x/2^n)^(2^n)
template <typename D>
D dpp_exp(D x)
{
  bool const neg(x < D(0));

  if (neg) x = -x;

  int n(0);

  while (x + x > D(1)) { x /= D(2); ++n; }

  D sum(1), term(1);

  for (int k = 1; k != 60; ++k)
  {
    term = term * x / D(k);
    sum += term;
  }

  for (int i = 0; i != n; ++i) sum *= sum;

  return neg ? D(1) / sum : sum;
}

// log(x): pull x towards 1 with repeated square roots, then
// log(x) = 2^n * 2 * atanh((x-1)/(x+1)); x must be positive
template <typename D>
D dpp_log(D x)
{
  int n(0);

  while (x * D(100) > D(101) || x * D(100) < D(99))
  {
    x = dpp::sqrt(x);
    ++n;
  }

  D const z((x - D(1)) / (x + D(1))), z2(z * z);

  D sum(z), term(z);

  for (int k = 1; k != 20; ++k)
  {
    term *= z2;
    sum += term / D(2 * k + 1);
  }

  return sum * D(1 << (n + 1));
}

// sin(x): Taylor series (the tests keep |x| below 4, 40 terms give
// x^81/81! truncation well below 1e-70)
template <typename D>
D dpp_sin(D x)
{
  D sum(x), term(x);

  for (int k = 1; k != 40; ++k)
  {
    term = term * -(x * x) / D((2 * k) * (2 * k + 1));
    sum += term;
  }

  return sum;
}

// cos(x): Taylor series
template <typename D>
D dpp_cos(D x)
{
  D sum(1), term(1);

  for (int k = 1; k != 40; ++k)
  {
    term = term * -(x * x) / D((2 * k - 1) * (2 * k));
    sum += term;
  }

  return sum;
}

// tan(x) = sin(x) / cos(x)
template <typename D>
D dpp_tan(D x)
{
  return dpp_sin(x) / dpp_cos(x);
}

// atan(x): halve the argument via atan(x) = 2*atan(x/(1+sqrt(1+x^2)))
// until it is at most 1/1000, then sum the Taylor series
template <typename D>
D dpp_atan(D x)
{
  bool const neg(x < D(0));

  if (neg) x = -x;

  int n(0);

  while (x * D(1000) > D(1))
  {
    x = x / (D(1) + dpp::sqrt(x * x + D(1)));
    ++n;
  }

  D const x2(x * x);

  D sum(x), term(x);

  for (int k = 1; k != 30; ++k)
  {
    term *= -x2;
    sum += term / D(2 * k + 1);
  }

  sum *= D(1 << n);

  return neg ? -sum : sum;
}

// hyperbolics via exp
template <typename D>
D dpp_sinh(D x)
{
  D const e(dpp_exp(x)), ie(D(1) / e);

  return (e - ie) / D(2);
}

template <typename D>
D dpp_cosh(D x)
{
  D const e(dpp_exp(x)), ie(D(1) / e);

  return (e + ie) / D(2);
}

template <typename D>
D dpp_tanh(D x)
{
  D const e(dpp_exp(x)), ie(D(1) / e);

  return (e - ie) / (e + ie);
}

// pow(a, b) = exp(b * log(a))
template <typename D>
D dpp_pow(D a, D b)
{
  return dpp_exp(b * dpp_log(a));
}

}

void test_elementary()
{
  SECTION("elementary functions");

  // exp: exp(1) is e (independent of the 1/n! series in the numeric
  // section), exp(1/2), exp(-1); checked against 30 digit bounds
  {
    auto const e_(dpp_exp(d128(1))), exp_half(dpp_exp(d128(1) / d128(2))),
      exp_m1(dpp_exp(-d128(1)));

    CHECK(e_ > 2.71828182845904523536028747135_d128);
    CHECK(e_ < 2.71828182845904523536028747136_d128);
    CHECK(exp_half > 1.64872127070012814684865078781_d128);
    CHECK(exp_half < 1.64872127070012814684865078782_d128);
    CHECK(exp_m1 > 0.367879441171442321595523770161_d128);
    CHECK(exp_m1 < 0.367879441171442321595523770162_d128);

    CHECK(std::fabs(double(e_) - std::exp(1.0)) < 1e-13);
    CHECK(std::fabs(double(exp_half) - std::exp(0.5)) < 1e-13);
    CHECK(std::fabs(double(exp_m1) - std::exp(-1.0)) < 1e-13);
  }

  // log: log(2), log(10), log(1.25)
  {
    auto const ln2_(dpp_log(d128(2))), ln10_(dpp_log(d128(10))),
      ln125(dpp_log(d128(5) / d128(4)));

    CHECK(ln2_ > 0.693147180559945309417232121458_d128);
    CHECK(ln2_ < 0.693147180559945309417232121459_d128);
    CHECK(ln10_ > 2.30258509299404568401799145468_d128);
    CHECK(ln10_ < 2.30258509299404568401799145469_d128);
    CHECK(ln125 > 0.223143551314209755766295090309_d128);
    CHECK(ln125 < 0.223143551314209755766295090310_d128);

    CHECK(std::fabs(double(ln2_) - std::log(2.0)) < 1e-13);
    CHECK(std::fabs(double(ln10_) - std::log(10.0)) < 1e-13);
    CHECK(std::fabs(double(ln125) - std::log(1.25)) < 1e-13);
  }

  // sin, cos, tan
  {
    auto const sin1_(dpp_sin(d128(1))), cos1_(dpp_cos(d128(1))),
      sin05(dpp_sin(d128(1) / d128(2))), cos05(dpp_cos(d128(1) / d128(2))),
      tan1_(dpp_tan(d128(1)));

    CHECK(sin1_ > 0.84147098480789650665250232163_d128);
    CHECK(sin1_ < 0.841470984807896506652502321631_d128);
    CHECK(cos1_ > 0.540302305868139717400936607442_d128);
    CHECK(cos1_ < 0.540302305868139717400936607443_d128);
    CHECK(sin05 > 0.479425538604203000273287935215_d128);
    CHECK(sin05 < 0.479425538604203000273287935216_d128);
    CHECK(cos05 > 0.877582561890372716116281582603_d128);
    CHECK(cos05 < 0.877582561890372716116281582604_d128);
    CHECK(tan1_ > 1.55740772465490223050697480745_d128);
    CHECK(tan1_ < 1.55740772465490223050697480746_d128);

    CHECK(std::fabs(double(sin1_) - std::sin(1.0)) < 1e-13);
    CHECK(std::fabs(double(cos1_) - std::cos(1.0)) < 1e-13);
    CHECK(std::fabs(double(tan1_) - std::tan(1.0)) < 1e-13);
  }

  // atan: 4*atan(1) is pi, atan(1/2)
  {
    auto const pi_(dpp_atan(d128(1)) * d128(4)),
      atan05(dpp_atan(d128(1) / d128(2)));

    CHECK(pi_ > 3.14159265358979323846264338327_d128);
    CHECK(pi_ < 3.14159265358979323846264338328_d128);
    CHECK(atan05 > 0.463647609000806116214256231461_d128);
    CHECK(atan05 < 0.463647609000806116214256231462_d128);

    CHECK(std::fabs(double(pi_) - 4.0 * std::atan(1.0)) < 1e-13);
    CHECK(std::fabs(double(atan05) - std::atan(0.5)) < 1e-13);
  }

  // sinh, cosh, tanh
  {
    auto const sinh1_(dpp_sinh(d128(1))), cosh1_(dpp_cosh(d128(1))),
      tanh1_(dpp_tanh(d128(1)));

    CHECK(sinh1_ > 1.17520119364380145688238185059_d128);
    CHECK(sinh1_ < 1.17520119364380145688238185060_d128);
    CHECK(cosh1_ > 1.54308063481524377847790562075_d128);
    CHECK(cosh1_ < 1.54308063481524377847790562076_d128);
    CHECK(tanh1_ > 0.761594155955764888119458282604_d128);
    CHECK(tanh1_ < 0.761594155955764888119458282605_d128);

    CHECK(std::fabs(double(sinh1_) - std::sinh(1.0)) < 1e-13);
    CHECK(std::fabs(double(cosh1_) - std::cosh(1.0)) < 1e-13);
    CHECK(std::fabs(double(tanh1_) - std::tanh(1.0)) < 1e-13);
  }

  // internal consistency at full precision
  {
    auto const e_(dpp_exp(d128(1))), s5(dpp_exp(d128(1) / d128(2))),
      em(dpp_exp(-d128(1)));

    CHECK(dpp::abs(s5 * s5 - e_) < .0000000000000000000000000000000001_d128);
    CHECK(dpp::abs(e_ * em - d128(1)) < .0000000000000000000000000000000001_d128);
  }
  {
    auto const s1(dpp_sin(d128(1))), c1(dpp_cos(d128(1)));

    CHECK(dpp::abs(s1 * s1 + c1 * c1 - d128(1)) <
      .0000000000000000000000000000000001_d128);
  }
  {
    auto const sh(dpp_sinh(d128(1))), ch(dpp_cosh(d128(1)));

    CHECK(dpp::abs(ch * ch - sh * sh - d128(1)) <
      .0000000000000000000000000000000001_d128);
  }

  // cos(2*atan(1)) == cos(pi/2) == 0
  CHECK(dpp::abs(dpp_cos(dpp_atan(d128(1)) * d128(2))) <
    .00000000000000000000000000000000001_d128);

  // round trips
  CHECK(dpp_exp(dpp_log(d128(2))) > 1.9999999999999999999999999999_d128);
  CHECK(dpp_exp(dpp_log(d128(2))) < 2.0000000000000000000000000001_d128);
  CHECK(dpp_log(dpp_exp(d128(1))) > 0.9999999999999999999999999999_d128);
  CHECK(dpp_log(dpp_exp(d128(1))) < 1.0000000000000000000000000001_d128);
  CHECK(dpp_atan(dpp_tan(d128(1) / d128(2))) > 0.4999999999999999999999999999_d128);
  CHECK(dpp_atan(dpp_tan(d128(1) / d128(2))) < 0.5000000000000000000000000001_d128);

  // pow: pow(2, 1/2) reproduces the square root of 2, pow(10, 2) == 100
  {
    auto const pow_half(dpp_pow(d128(2), d128(1) / d128(2))),
      p2(dpp_pow(d128(10), d128(2)));

    CHECK(pow_half > 1.4142135623730950488016887242_d128);
    CHECK(pow_half < 1.41421356237309504880168872421_d128);
    CHECK(p2 > 99.9999999999999999999999999999_d128);
    CHECK(p2 < 100.0000000000000000000000000001_d128);
  }

  // d256 spot checks, same bounds (intt significand arithmetic)
  {
    auto const e256(dpp_exp(d256(1))), ln2_256(dpp_log(d256(2))),
      sin1_256(dpp_sin(d256(1))), pi_256(dpp_atan(d256(1)) * d256(4));

    CHECK(e256 > 2.71828182845904523536028747135_d256);
    CHECK(e256 < 2.71828182845904523536028747136_d256);
    CHECK(ln2_256 > 0.693147180559945309417232121458_d256);
    CHECK(ln2_256 < 0.693147180559945309417232121459_d256);
    CHECK(sin1_256 > 0.84147098480789650665250232163_d256);
    CHECK(sin1_256 < 0.841470984807896506652502321631_d256);
    CHECK(pi_256 > 3.14159265358979323846264338327_d256);
    CHECK(pi_256 < 3.14159265358979323846264338328_d256);
  }

  // d64 spot checks with wider bounds (integral significand arithmetic)
  {
    auto const e64(dpp_exp(d64(1))), sin1_64(dpp_sin(d64(1)));

    CHECK(e64 > 2.718281828459045_d64);
    CHECK(e64 < 2.718281828459046_d64);
    CHECK(sin1_64 > 0.8414709848078965_d64);
    CHECK(sin1_64 < 0.8414709848078966_d64);
  }
}

//////////////////////////////////////////////////////////////////////////////
// rounding-function semantics beyond the basics
//////////////////////////////////////////////////////////////////////////////
void test_rounding_semantics()
{
  SECTION("rounding semantics: half-away-from-zero, edges, identities");

  // round() is round-half-away-from-zero (a +/- 0.5, then trunc), NOT
  // round-half-to-even: every x.5 tie below rounds away from zero
  CHECK_EQ(dpp::round(d32(direct, 5, -1)), d32(1));    //  0.5 ->  1
  CHECK_EQ(dpp::round(d32(direct, 15, -1)), d32(2));   //  1.5 ->  2
  CHECK_EQ(dpp::round(d32(direct, 25, -1)), d32(3));   //  2.5 ->  3 (banker's: 2)
  CHECK_EQ(dpp::round(d32(direct, 35, -1)), d32(4));   //  3.5 ->  4
  CHECK_EQ(dpp::round(d32(direct, 45, -1)), d32(5));   //  4.5 ->  5 (banker's: 4)
  CHECK_EQ(dpp::round(d32(direct, -5, -1)), d32(-1));  // -0.5 -> -1
  CHECK_EQ(dpp::round(d32(direct, -25, -1)), d32(-3)); // -2.5 -> -3 (banker's: -2)
  CHECK_EQ(dpp::round(d32(direct, -35, -1)), d32(-4)); // -3.5 -> -4
  CHECK_EQ(dpp::round(d16(direct, 25, -1)), d16(3));   // same contract at d16
  CHECK_EQ(dpp::round(d32(direct, 4999, -4)), d32(0)); // 0.4999 below the tie
  CHECK_EQ(dpp::round(d32(direct, 50001, -5)), d32(1)); // 0.50001 above the tie

  // already-integral values pass through unchanged: exp >= 0 is returned
  // as-is (same significand/exponent), a trailing fraction is canonicalized
  {
    auto const r(dpp::round(d32(direct, 123, 5)));

    CHECK_EQ(r.sig(), 123);
    CHECK_EQ(r.exp(), 5);
  }
  CHECK_EQ(dpp::round(d32(direct, 200, -2)), d32(2));
  CHECK_EQ(dpp::round(d32(direct, -200, -2)), d32(-2));

  // NaN propagates through trunc/ceil/floor (round() would too, but its
  // implementation consults the SIGNIFICAND of NaN before any isnan guard,
  // i.e. it reads the indeterminate NaN significand: benign for the result
  // yet formally UB, so it is deliberately not exercised here)
  CHECK(dpp::isnan(dpp::trunc(d32(dpp::nan))));
  CHECK(dpp::isnan(dpp::ceil(d32(dpp::nan))));
  CHECK(dpp::isnan(dpp::floor(d32(dpp::nan))));

  // truncation towards the canonical zero
  CHECK_EQ(dpp::trunc(d32(direct, -5, -1)), d32(0));  // -0.5 -> 0 (no -0)
  CHECK_EQ(dpp::trunc(d32(direct, -15, -1)), d32(-1));

  // ceil/floor halves and already-integral values with negative exponents
  CHECK_EQ(dpp::ceil(d32(direct, 5, -1)), d32(1));
  CHECK_EQ(dpp::ceil(d32(direct, -5, -1)), d32(0));
  CHECK_EQ(dpp::floor(d32(direct, 5, -1)), d32(0));
  CHECK_EQ(dpp::floor(d32(direct, -5, -1)), d32(-1));
  CHECK_EQ(dpp::ceil(d32(direct, 200, -2)), d32(2));
  CHECK_EQ(dpp::ceil(d32(direct, -200, -2)), d32(-2));
  CHECK_EQ(dpp::floor(d32(direct, 200, -2)), d32(2));
  CHECK_EQ(dpp::floor(d32(direct, -200, -2)), d32(-2));

  // identities: ceil(x) == -floor(-x), floor(x) <= x <= ceil(x) and
  // trunc(x) == floor(x)/ceil(x) according to the sign
  for (auto [m, e] : {std::pair{25, -1}, {35, -1}, {-25, -1}, {-35, -1},
    {1, -5}, {-1, -5}, {271828182, -8}, {-314159265, -8}, {200, -2}, {-200, -2}})
  {
    d32 const v(direct, m, e), f(dpp::floor(v)), c(dpp::ceil(v));

    CHECK(c == -dpp::floor(-v));
    CHECK(f <= v);
    CHECK(v <= c);
    CHECK(dpp::trunc(v) == (v < d32(0) ? c : f));
  }

  // all four are constexpr-friendly
  static_assert(dpp::round(d32(direct, 25, -1)) == d32(3));
  static_assert(dpp::trunc(d32(direct, 25, -1)) == d32(2));
  static_assert(dpp::ceil(d32(direct, 15, -1)) == d32(2));
  static_assert(dpp::floor(d32(direct, 15, -1)) == d32(1));
  static_assert(dpp::round(d32(direct, -25, -1)) == d32(-3));
}

//////////////////////////////////////////////////////////////////////////////
// increment/decrement, boolean conversion, nan_t comparison, x / 0
//////////////////////////////////////////////////////////////////////////////
void test_increment_and_boolean()
{
  SECTION("++, --, operator bool, == nan_t, division by zero");

  // pre/post forms add/subtract exactly 1 and yield the proper value
  d32 x(direct, 25, -1);

  CHECK_EQ(x++, d32(direct, 25, -1));
  CHECK_EQ(x, d32(direct, 35, -1));
  CHECK_EQ(x--, d32(direct, 35, -1));
  CHECK_EQ(x, d32(direct, 25, -1));
  CHECK_EQ(--x, d32(direct, 15, -1));
  CHECK_EQ(++x, d32(direct, 25, -1));
  CHECK_EQ(++d32(direct, -5, -1), d32(direct, 5, -1)); // -0.5 -> 0.5
  CHECK_EQ(--d32(direct, 5, -1), d32(direct, -5, -1)); // 0.5 -> -0.5

  // NaN stays NaN under ++/-- and compound assignment
  {
    d32 n(dpp::nan);

    CHECK(dpp::isnan(n++));
    CHECK(dpp::isnan(n));
    CHECK(dpp::isnan(--n));
    CHECK(dpp::isnan(n));
    CHECK(dpp::isnan(n += 1));
  }
  {
    d24 n(dpp::nan);

    CHECK(dpp::isnan(n++));
    CHECK(dpp::isnan(n--));
    CHECK(dpp::isnan(n));
  }

  // incrementing the largest d16 value absorbs the 1 (beyond precision)
  {
    d16 m(d16::max);

    ++m;
    CHECK(!dpp::isnan(m));
    CHECK_EQ(m, d16::max);
  }

  // constexpr-friendliness
  static_assert([]{ d32 y(2); ++y; return y == d32(3); }());
  static_assert(dpp::isnan([]{ d32 y(dpp::nan); ++y; return y; }()));

  // bool: false only for the canonical zero; NaN is truthy
  CHECK(!static_cast<bool>(d32(0)));
  CHECK(!static_cast<bool>(d32(direct, 0, -5)));
  CHECK(static_cast<bool>(d32(-1)));
  CHECK(static_cast<bool>(d32::eps));
  CHECK(static_cast<bool>(d32(dpp::nan)));
  static_assert(!static_cast<bool>(d32(0)));
  static_assert(static_cast<bool>(d32(dpp::nan)));

  // x == dpp::nan uses the nan_t overload: true iff x is NaN
  CHECK(!(d32(1) == dpp::nan));
  CHECK(d32(dpp::nan) == dpp::nan);
  CHECK(d16(dpp::nan) == dpp::nan);
  CHECK(d32(1) != dpp::nan);
  CHECK(!(d32(dpp::nan) != dpp::nan));
  static_assert(!(d32(1) == dpp::nan));
  static_assert(d32(dpp::nan) == dpp::nan);

  // division by zero is NaN (the library has no infinities)
  CHECK(dpp::isnan(d32(1) / d32(0)));
  CHECK(dpp::isnan(d32(0) / d32(0)));
  CHECK(dpp::isnan(d32(-1) / d32(0)));
  CHECK(dpp::isnan(d32::max / d32(0)));
  CHECK(dpp::isnan(d24(1) / d24(0)));
  CHECK(dpp::isnan(d32(1) / d32(direct, 0, -5))); // non-canonical zero
  CHECK(dpp::isnan(d32(dpp::nan) / d32(0)));
  static_assert(dpp::isnan(d32(1) / d32(0)));
  static_assert(dpp::isnan(d24(1) / d24(0)));

  // compound assignment with native right-hand sides
  {
    d32 c(1);

    c += 2;   CHECK_EQ(c, d32(3));
    c *= 3;   CHECK_EQ(c, d32(9));
    c -= 4;   CHECK_EQ(c, d32(5));
    c /= 4;   CHECK_EQ(c, d32(direct, 125, -2));
    c += 0.5; CHECK_EQ(c, d32(direct, 175, -2));
  }
  {
    d32 w(0);

    w += d64(direct, 5, -1); // cross-type compound narrows via the ctor
    CHECK_EQ(w, d32(direct, 5, -1));
  }
}

//////////////////////////////////////////////////////////////////////////////
// fma and midpoint semantics
//////////////////////////////////////////////////////////////////////////////
void test_fused_midpoint_semantics()
{
  SECTION("fma (single rounding) and midpoint");

  // fma computes a*b+c with ONE rounding at the end: the product is kept in
  // the doubled accumulator, so fma recovers the exact tail that two-step
  // arithmetic loses to intermediate rounding
  {
    d16 const a(9999), c(direct, -9998, 4); // c = -(the d16-rounded a*a)

    CHECK_EQ(a * a, d16(direct, 9998, 4)); // 99980001 rounds to 9998e4
    CHECK_EQ(a * a + c, d16(0));           // two-step: tail lost
    CHECK_EQ(dpp::fma(a, a, c), d16(1));   // fused: exact tail 1
  }
  {
    // d32: a*a = 4611686014132420609 (19 digits); the two-step product
    // rounds to 461168601e10 and the fused tail is exactly 4132420609
    d32 const a(direct, 2147483647, 0), c(direct, -461168601, 10);

    CHECK_EQ(a * a, d32(direct, 461168601, 10));
    CHECK_EQ(a * a + c, d32(0));
    CHECK_EQ(dpp::fma(a, a, c), d32(direct, 413242061, 1));
  }

  // alignment and parity across the exponent branches (exact decimal values)
  CHECK_EQ(dpp::fma(d32(direct, 5, -1), d32(direct, 3, 0), d32(direct, 1, 0)),
    d32(direct, 25, -1));  // 0.5*3+1 = 2.5 (product exponent below c's)
  CHECK_EQ(dpp::fma(d32(direct, 5, -2), d32(direct, 3, 0), d32(direct, 1, 0)),
    d32(direct, 115, -2)); // 0.05*3+1 = 1.15 (alignment shift of two)
  CHECK_EQ(dpp::fma(d32(direct, 5, -1), d32(direct, 3, 0), d32(direct, -1, 0)),
    d32(direct, 5, -1));   // 0.5*3-1 = 0.5 (negative c)
  CHECK_EQ(dpp::fma(d32(direct, 3, -1), d32(direct, 3, -1), d32(direct, 1, -2)),
    d32(direct, 1, -1));   // 0.3*0.3+0.01 = 0.1 (equal exponents)
  CHECK_EQ(dpp::fma(d32(direct, 3, 0), d32(direct, 3, 0), d32(direct, 5, -1)),
    d32(direct, 95, -1));  // 3*3+0.5 = 9.5 (product exponent above c's)
  CHECK_EQ(dpp::fma(d32(direct, 2, 0), d32(direct, 5, -1), d32(direct, 1, -1)),
    d32(direct, 11, -1));  // 2*0.5+0.1 = 1.1
  CHECK_EQ(dpp::fma(d32(direct, 3, 0), d32(direct, 5, -1), d32(direct, 5, -1)),
    d32(2));               // 3*0.5+0.5 = 2
  CHECK_EQ(dpp::fma(d32(direct, 333333333, -9), d32(direct, 3, 0),
    d32(direct, 0, 0)), d32(direct, 999999999, -9)); // (1/3)*3 + 0

  // zero and NaN operands
  CHECK_EQ(dpp::fma(d32(0), d32(5), d32(7)), d32(7));
  CHECK_EQ(dpp::fma(d32(5), d32(0), d32(7)), d32(7));
  CHECK_EQ(dpp::fma(d32(5), d32(0), d32(0)), d32(0));
  CHECK(dpp::isnan(dpp::fma(d32(dpp::nan), d32(1), d32(1))));
  CHECK(dpp::isnan(dpp::fma(d32(1), d32(dpp::nan), d32(1))));
  CHECK(dpp::isnan(dpp::fma(d32(1), d32(1), d32(dpp::nan))));
  static_assert(dpp::fma(d32(2), d32(3), d32(4)) == d32(10));
  static_assert(dpp::fma(d16(9999), d16(9999), d16(direct, -9998, 4)) == d16(1));
  static_assert(dpp::isnan(dpp::fma(d32(1), d32(1), d32(dpp::nan))));

  // midpoint: the 5*(a+b) product of the two largest values needs one more
  // exponent step than emax allows, so (unlike an IEEE-style midpoint) it is
  // NaN even though the exact midpoint (== max) is representable
  CHECK(dpp::isnan(dpp::midpoint(d16::max, d16::max)));
  CHECK(dpp::isnan(dpp::midpoint(d32::max, d32::max)));
  CHECK(dpp::isnan(dpp::midpoint(d64::max, d64::max)));
  CHECK(dpp::isnan((d16::max + d16::max) / d16(2))); // the naive form too

  // opposite signs cancel in the widened accumulator: no overflow
  CHECK_EQ(dpp::midpoint(d16::max, d16::min), d16(0));
  CHECK_EQ(dpp::midpoint(d32::max, d32::min), d32(0));

  // exact halves (odd sums) and exact values
  CHECK_EQ(dpp::midpoint(d32(1), d32(2)), d32(direct, 15, -1));
  CHECK_EQ(dpp::midpoint(d32(1), d32(3)), d32(2));
  CHECK_EQ(dpp::midpoint(d32(1), d32(4)), d32(direct, 25, -1));
  CHECK_EQ(dpp::midpoint(d32(1), d32(6)), d32(direct, 35, -1));
  CHECK_EQ(dpp::midpoint(d32(direct, 1, -1), d32(direct, 2, -1)),
    d32(direct, 15, -2)); // 0.15
  CHECK_EQ(dpp::midpoint(d32(0), d32::eps), d32(direct, 5, -10));
  CHECK_EQ(dpp::midpoint(d16(0), d16::eps), d16(direct, 5, -5));

  // agrees with a + (b - a) / 2 away from the extremes
  CHECK_EQ(dpp::midpoint(d32(1), d32(4)),
    d32(1) + (d32(4) - d32(1)) / d32(2));
  CHECK_EQ(dpp::midpoint(d32(direct, 1, -1), d32(direct, 2, -1)),
    d32(direct, 1, -1) + (d32(direct, 2, -1) - d32(direct, 1, -1)) / d32(2));

  // NaN propagation
  CHECK(dpp::isnan(dpp::midpoint(d32(dpp::nan), d32(1))));
  CHECK(dpp::isnan(dpp::midpoint(d32(1), d32(dpp::nan))));
  static_assert(dpp::midpoint(d32(1), d32(2)) == d32(direct, 15, -1));
  static_assert(dpp::isnan(dpp::midpoint(d32(dpp::nan), d32(1))));
}

//////////////////////////////////////////////////////////////////////////////
// inv (reciprocal) semantics
//////////////////////////////////////////////////////////////////////////////
void test_inv_semantics()
{
  SECTION("inv: exact reciprocals, eps, underflow, NaN");

  // exact decimal reciprocals (the 2*5 = 10 family and powers of ten)
  CHECK_EQ(dpp::inv(d32(2)), d32(direct, 5, -1));
  CHECK_EQ(dpp::inv(d32(4)), d32(direct, 25, -2));
  CHECK_EQ(dpp::inv(d32(5)), d32(direct, 2, -1));
  CHECK_EQ(dpp::inv(d32(8)), d32(direct, 125, -3));
  CHECK_EQ(dpp::inv(d32(direct, 5, -1)), d32(2));
  CHECK_EQ(dpp::inv(d32(direct, 25, -2)), d32(4));
  CHECK_EQ(dpp::inv(d32(direct, 125, -3)), d32(8));
  CHECK_EQ(dpp::inv(d32(direct, 2, -1)), d32(5));
  CHECK_EQ(dpp::inv(d32(1)), d32(1));
  CHECK_EQ(dpp::inv(d32(-1)), d32(-1));
  CHECK_EQ(dpp::inv(d16(2)), d16(direct, 5, -1));

  // repeating reciprocals match the division result digit for digit
  CHECK_EQ(dpp::inv(d32(3)), d32(1) / d32(3));
  CHECK_EQ(dpp::inv(d32(3)), d32(direct, 333333333, -9));
  CHECK_EQ(dpp::inv(d64(3)), d64(1) / d64(3));
  CHECK_EQ(dpp::inv(d64(3)), d64(direct, 3333333333333333333LL, -19));
  CHECK_EQ(dpp::inv(d32(direct, 3, -1)),
    d32(direct, 1, 0) / d32(direct, 3, -1));
  CHECK_EQ(dpp::inv(dpp::inv(d32(3))), d32(3));
  CHECK_EQ(dpp::inv(dpp::inv(d64(3))), d64(3));

  // inverting eps gives the largest power-of-ten step
  CHECK_EQ(dpp::inv(d16::eps), d16(direct, 1, 4));
  CHECK_EQ(dpp::inv(d32::eps), d32(direct, 1, 9));
  CHECK_EQ(dpp::inv(d64::eps), d64(direct, 1, 18));

  // reciprocals of the largest values underflow: the emin-lifting loop
  // divides the significand down to zero -> canonical zero (flush to zero)
  CHECK_EQ(dpp::inv(d16::max), d16(0));
  CHECK_EQ(dpp::inv(d32::max), d32(0));
  CHECK_EQ(d16(direct, 1, 0) / d16::max, d16(0));
  CHECK_EQ(d32(direct, 1, 0) / d32::max, d32(0));

  // sign symmetry and degenerate operands
  CHECK_EQ(dpp::inv(d32(-3)), -dpp::inv(d32(3)));
  CHECK(dpp::isnan(dpp::inv(d32(0))));
  CHECK(dpp::isnan(dpp::inv(d24(0))));
  CHECK(dpp::isnan(dpp::inv(d32(dpp::nan))));
  CHECK(dpp::isnan(dpp::inv(d24(dpp::nan))));
  static_assert(dpp::inv(d32(4)) == d32(direct, 25, -2));
  static_assert(dpp::inv(d32(direct, 2, -1)) == d32(5));
  static_assert(dpp::isnan(dpp::inv(d32(0))));
}

//////////////////////////////////////////////////////////////////////////////
// double <-> dpp interop and exact decimal parsing
//////////////////////////////////////////////////////////////////////////////
void test_binary_interop_semantics()
{
  SECTION("double ctor, round trips, to_decimal truncation");

  // the double ctor converts the binary value at the decimal granularity
  // implied by its binary exponent scale (15..17 digits at d64): for
  // 0.1/0.2/0.3 the binary noise (scale 1e-21) rounds away and the clean
  // decimal values come out
  CHECK_EQ(dpp::to_string(d64(0.1)), "0.1");
  CHECK_EQ(dpp::to_string(d64(0.2)), "0.2");
  CHECK_EQ(dpp::to_string(d64(0.3)), "0.3");
  CHECK(d64(0.1) == d64(direct, 1, -1));
  CHECK_EQ(dpp::to_string(d64(1e-9)), "0.000000001");
  CHECK_EQ(dpp::to_string(d64(1e8)), "100000000");
  CHECK(d64(1e8) == d64(direct, 1, 8));
  CHECK_EQ(dpp::to_string(d64(6.02214076e23)), "602214076000000000000000");

  // other values keep a decimal trace of the binary noise, so the round
  // trip through dpp does not return the original double
  CHECK_EQ(dpp::to_string(d64(1.0 / 3.0)), "0.3333333333333333");
  CHECK(static_cast<double>(d64(1.0 / 3.0)) == 1.0 / 3.0);
  CHECK_EQ(dpp::to_string(d64(2.0 / 3.0)), "0.666666666666667");
  CHECK(static_cast<double>(d64(2.0 / 3.0)) != 2.0 / 3.0);
  CHECK_EQ(dpp::to_string(d64(0.30000000000000004)), "0.3000000000000001");
  CHECK(static_cast<double>(d64(0.30000000000000004)) != 0.30000000000000004);
  CHECK_EQ(dpp::to_string(d64(123456789.123456789)), "123456789.1234568");
  CHECK(static_cast<double>(d64(123456789.123456789)) != 123456789.123456789);

  // 0.1 + 0.2 == 0.3 holds for the double ctor too (noise below precision)
  CHECK(d64(0.1) + d64(0.2) == d64(0.3));

  // non-finite and negative-zero doubles
  CHECK(dpp::isnan(d32(INFINITY)));
  CHECK(dpp::isnan(d32(-INFINITY)));
  CHECK(dpp::isnan(d32(NAN)));
  CHECK_EQ(d32(-0.0), d32(0));

  // dpp -> double: exact values round trip; exponent extremes underflow to
  // zero / overflow to infinity
  CHECK(static_cast<double>(d64(0.5)) == 0.5);
  CHECK(static_cast<double>(d64(direct, 1, -1)) == 0.1);
  CHECK_EQ(static_cast<double>(d128(direct, 1, -4000)), 0.0);
  CHECK(std::isinf(static_cast<double>(d128(direct, 1, 4000))));

  // exact decimal arithmetic: accumulating the parsed 0.1 ten times gives
  // exactly 1 at every width (the famous binary floating point failure)
  {
    d16 s(0);

    for (int i = 0; i != 10; ++i) s += 0.1_d16;

    CHECK_EQ(s, d16(1));
  }
  {
    d32 s(0);

    for (int i = 0; i != 10; ++i) s += 0.1_d32;

    CHECK_EQ(s, d32(1));
  }
  {
    d64 s(0);

    for (int i = 0; i != 10; ++i) s += 0.1_d64;

    CHECK_EQ(s, d64(1));
  }
  {
    d128 s(0);

    for (int i = 0; i != 10; ++i) s += 0.1_d128;

    CHECK_EQ(s, d128(1));
  }
  CHECK_EQ(0.1_d32 + 0.2_d32, 0.3_d32);
  CHECK(0.1_d64 + 0.2_d64 == 0.3_d64);

  // to_decimal accepts plain decimal digits only (no scientific notation)
  // and TRUNCATES (never rounds) once the significand is full
  CHECK_EQ(dpp::to_decimal<d16>("0.12345"), d16(direct, 12345, -5));
  CHECK_EQ(dpp::to_decimal<d16>("0.123456789"), d16(direct, 12345, -5));
  CHECK_EQ(dpp::to_decimal<d16>("0.300005"), d16(direct, 3, -1)); // '5' dropped
  CHECK_EQ(dpp::to_decimal<d16>("99999999999"), d16(direct, 9999, 7));
  CHECK_EQ(dpp::to_decimal<d64>("1e8"), d64(1)); // stops at 'e'
  CHECK_EQ(dpp::to_decimal<d64>("1.5e-3"), d64(direct, 15, -1));

  // the dpp(U, e) constructor rounds wide integers through a doubled
  // accumulator with a single final division: exact when the input carries
  // a trailing zero, otherwise it sheds one more digit than necessary
  CHECK_EQ(d32(21474836470LL), d32(direct, 214748365, 2)); // 11 digits -> 9
  CHECK_EQ(d32(10000000000LL), d32(direct, 1, 10));        // 1e10 exact
  CHECK_EQ(d32(-21474836480LL), d32(direct, -214748365, 2));
}

//////////////////////////////////////////////////////////////////////////////
// numerical analysis: quadratic cancellation + Mandelbrot orbits
//////////////////////////////////////////////////////////////////////////////
void test_quadratic_and_mandelbrot()
{
  SECTION("quadratic cancellation and Mandelbrot orbits");

  // x^2 - B x + 1 = 0 with B = 1e8: the small root suffers catastrophic
  // cancellation in the textbook formula; the reciprocal form keeps all
  // digits. The stable identities below hold EXACTLY
  {
    d64 const B(direct, 100000000, 0);
    d64 const disc(B * B - d64(direct, 4, 0));

    CHECK_EQ(dpp::to_string(disc), "9999999999999996");

    d64 const r(dpp::sqrt(disc));

    CHECK_EQ(dpp::to_string(r), "99999999.99999998");

    d64 const x1((B + r) / d64(direct, 2, 0));
    d64 const x2_naive((B - r) / d64(direct, 2, 0));
    d64 const x2_stable(d64(direct, 1, 0) / x1);
    auto const p([&](d64 const& x) { return x * x - B * x + d64(direct, 1, 0); });

    CHECK_EQ(dpp::to_string(x1), "99999999.99999999");
    CHECK_EQ(dpp::to_string(x2_naive), "0.00000001");
    CHECK_EQ(dpp::to_string(x2_stable), "0.000000010000000000000001");
    CHECK(x2_stable != x2_naive);               // cancellation is visible
    CHECK_EQ(p(x2_naive), d64(direct, 1, -16)); // textbook error = 1e-16
    CHECK_EQ(p(x2_stable), d64(0));             // stable root is EXACT
    CHECK_EQ(x1 * x2_stable, d64(1));           // Vieta product exact
    CHECK_EQ(x1 + x2_stable, B);                // Vieta sum exact
  }

  // same equation at d128: 34 digits, cross-checked against python decimal
  {
    d128 const B(direct, 100000000, 0);
    d128 const r(dpp::sqrt(B * B - d128(direct, 4, 0)));
    d128 const x1((B + r) / d128(direct, 2, 0));
    d128 const x2(d128(direct, 1, 0) / x1);

    CHECK_EQ(dpp::to_string(x1), "99999999.999999989999999999999999");
    CHECK_EQ(dpp::to_string(x2),
      "0.0000000100000000000000010000000000000002");
  }

  // Mandelbrot escape-time iteration z = z^2 + c in exact decimal arithmetic
  {
    using C = std::pair<d32, d32>;
    auto const step([&](C const& z, C const& c)
      {
        return C{z.first * z.first - z.second * z.second + c.first,
          z.first * z.second + z.second * z.first + c.second};
      });
    d32 const zero(direct, 0, 0);

    // c = -1: the orbit is exactly 0, -1, 0, -1, ... forever
    {
      C z{zero, zero};
      C const c{d32(direct, -1, 0), zero};
      bool exact{true};

      for (int i = 0; i != 31; ++i)
      {
        z = step(z, c);

        if (z.first != ((i % 2) ? zero : d32(direct, -1, 0)) ||
          z.second != zero) exact = false;
      }

      CHECK(exact);
    }

    // c = -2: 0 -> -2 -> 2 -> 2 -> ... (exact fixed point)
    {
      C z{zero, zero};
      C const c{d32(direct, -2, 0), zero};

      z = step(z, c);
      CHECK_EQ(z.first, d32(direct, -2, 0));
      z = step(z, c);
      CHECK_EQ(z.first, d32(2));
      z = step(z, c);
      CHECK_EQ(z.first, d32(2));
      CHECK_EQ(z.second, zero);
    }

    // c = 0.25 (parabolic boundary): z creeps monotonically towards 0.5
    // and stays below it for 50 iterations
    {
      C z{zero, zero};
      C const c{d32(direct, 25, -2), zero};
      d32 prev(zero);
      bool mono{true}, bnd{true};

      for (int i = 0; i != 50; ++i)
      {
        z = step(z, c);

        if (!(z.first > prev)) mono = false;
        if (!(z.first < d32(direct, 5, -1))) bnd = false;

        prev = z.first;
      }

      CHECK(mono);
      CHECK(bnd);
      CHECK(z.first > d32(direct, 482, -3)); // 0.482 < z50 < 0.483
      CHECK(z.first < d32(direct, 483, -3));
    }

    // escaping points with exact orbit values
    {
      C z{zero, zero};
      C const c{d32(direct, 3, 0), zero};

      z = step(z, c);
      z = step(z, c);
      CHECK_EQ(z.first, d32(12)); // 0 -> 3 -> 12
    }
    {
      C z{zero, zero};
      C const c{d32(direct, -21, -1), zero};
      d32 const expect[]{d32(direct, -21, -1), d32(direct, 231, -2),
        d32(direct, 32361, -4), d32(direct, 837234321, -8)};

      for (int i = 0; i != 4; ++i)
      {
        z = step(z, c);
        CHECK_EQ(z.first, expect[i]);
        CHECK_EQ(z.second, zero);
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
// factorial exactness boundaries
//////////////////////////////////////////////////////////////////////////////
void test_factorial_exactness()
{
  SECTION("factorial exactness boundaries");

  // d32 (10 digit significand capacity): 15! is the last exact factorial;
  // 16! and beyond acquire rounding in their trailing digits
  {
    d32 f(1);

    for (int i = 1; i != 13; ++i) f *= d32(i);

    CHECK_EQ(f, d32(479001600)); // 12!
    f *= d32(13); CHECK_EQ(f, d32(6227020800));
    f *= d32(14); CHECK_EQ(f, d32(87178291200));
    f *= d32(15); CHECK_EQ(f, d32(1307674368000)); // last exact: 15!
    f *= d32(16); // true value 20922789888000, rounded in the last digits
    CHECK_EQ(dpp::to_string(f), "20922789890000");
    f *= d32(17); // true value 355687428096000
    CHECK_EQ(dpp::to_string(f), "355687428000000");
    f *= d32(18); // true value 6402373705728000
    CHECK_EQ(dpp::to_string(f), "6402373700000000");
  }

  // d64 (19 digit significand capacity): 23! is the last exact factorial
  {
    d64 f(1);

    for (int i = 1; i != 21; ++i) f *= d64(i);

    CHECK_EQ(f, d64(2432902008176640000LL)); // 20!
    f *= d64(21); CHECK_EQ(f, d64(direct, 5109094217170944000LL, 1));
    f *= d64(22); CHECK_EQ(f, d64(direct, 1124000727777607680LL, 3));
    f *= d64(23); // last exact
    CHECK_EQ(f, d64(direct, 2585201673888497664LL, 4));
    f *= d64(24); // true value 620448401733239439360000
    CHECK_EQ(dpp::to_string(f), "620448401733239439400000");
    f *= d64(25); // true value 15511210043330985984000000
    CHECK_EQ(dpp::to_string(f), "15511210043330985990000000");
    f *= d64(26); // true value 403291461126605635584000000
    CHECK_EQ(dpp::to_string(f), "403291461126605635700000000");
  }

  // exact within constant evaluation too
  static_assert([]{ d32 f(1); for (int i = 2; i != 11; ++i) f *= d32(i);
    return f == d32(3628800); }()); // 10!
}

//////////////////////////////////////////////////////////////////////////////
// safe integral conversion (to_integral.hpp)
//////////////////////////////////////////////////////////////////////////////
void test_to_integral()
{
  SECTION("to_integral");

  {
    auto const r = dpp::to_integral(d32(42));
    CHECK_EQ(r.first, 42);
    CHECK(!r.second);
  }
  {
    auto const r = dpp::to_integral(d64(1234567890123LL));
    CHECK_EQ(r.first, 1234567890123LL);
    CHECK(!r.second);
  }
  {
    // fractional values truncate toward zero
    auto const r = dpp::to_integral(d32(1.5));
    CHECK_EQ(r.first, 1);
    CHECK(!r.second);

    auto const r2 = dpp::to_integral(d32(-1.5));
    CHECK_EQ(r2.first, -1);
    CHECK(!r2.second);
  }
  {
    // positive exponents are multiplied out
    auto const r = dpp::to_integral(d32(direct, 123, 2));
    CHECK_EQ(r.first, 12300);
    CHECK(!r.second);

    auto const r2 = dpp::to_integral(d64(direct, 123, 2));
    CHECK_EQ(r2.first, 12300);
    CHECK(!r2.second);
  }
  {
    // NaN
    auto const r = dpp::to_integral(d32(dpp::nan));
    CHECK_EQ(r.first, 0);
    CHECK(r.second);
  }
  {
    // overflow is reported via the flag
    auto const r = dpp::to_integral<std::int8_t>(d32(1000));
    CHECK_EQ(r.first, 0);
    CHECK(r.second);

    auto const r2 = dpp::to_integral<std::int16_t>(d32(40000));
    CHECK_EQ(r2.first, 0);
    CHECK(r2.second);

    auto const r3 = dpp::to_integral<std::int16_t>(d32(30000));
    CHECK_EQ(r3.first, 30000);
    CHECK(!r3.second);
  }
  {
    // intmax_t boundaries
    auto const r = dpp::to_integral(d64(9223372036854775807LL));
    CHECK_EQ(r.first, 9223372036854775807LL);
    CHECK(!r.second);

    auto const r2 = dpp::to_integral(d64(1000000000000000000LL));
    CHECK_EQ(r2.first, 1000000000000000000LL);
    CHECK(!r2.second);
  }
}

//////////////////////////////////////////////////////////////////////////////
// fast division utilities (utils.hpp)
//////////////////////////////////////////////////////////////////////////////
void test_fast_div()
{
  SECTION("fast division utilities");

  CHECK_EQ(dpp::div2(5_d32), d32(2.5));
  CHECK_EQ(dpp::div2(1_d64), d64(0.5));
  CHECK_EQ(dpp::div2(-5_d32), d32(-2.5));
  CHECK_EQ(dpp::div4(1_d32), d32(0.25));
  CHECK_EQ(dpp::div4(3_d32), d32(0.75));
  CHECK_EQ(dpp::div5(1_d32), d32(0.2));
  CHECK_EQ(dpp::div5(3_d32), d32(0.6));
  CHECK_EQ(dpp::div8(1_d32), d32(0.125));
  CHECK_EQ(dpp::div8(7_d64), d64(0.875));
  CHECK_EQ(dpp::div10(1_d32), d32(0.1));
  CHECK_EQ(dpp::div10<2>(1_d32), d32(0.01));
  CHECK_EQ(dpp::div10<3>(1_d64), d64(0.001));
  CHECK_EQ(dpp::div10(-1_d32), d32(-0.1));

  CHECK_EQ(dpp::mul2(d32(0.5)), d32(1));
  CHECK_EQ(dpp::mul2(d64(1.5)), d64(3));
  CHECK_EQ(dpp::mul2(-1_d64), d64(-2));
  CHECK_EQ(dpp::mul2<2>(d32(0.25)), d32(1));
  CHECK_EQ(dpp::mul2<3>(d32(0.125)), d32(1));

  // div<B> goes through the reciprocal
  CHECK_EQ(dpp::div<4>(1_d32), d32(0.25));
  CHECK_EQ(dpp::div<8>(1_d64), d64(0.125));
  CHECK(dpp::abs(dpp::div<3>(1_d32) * d32(3) - d32(1)) <= d32::eps);
  CHECK(dpp::abs(dpp::div<3>(1_d64) * d64(3) - d64(1)) <= d64::eps);

  // equivalence with the division operator
  CHECK_EQ(dpp::div2(d64(12345)), d64(12345) / d64(2));
  CHECK_EQ(dpp::div5(d64(12345)), d64(12345) / d64(5));
  CHECK_EQ(dpp::div10<2>(d64(12345)), d64(12345) / d64(100));
  CHECK_EQ(dpp::mul2(d64(12345)), d64(12345) * d64(2));

  // NaN propagation
  CHECK(dpp::isnan(dpp::div2(d32(dpp::nan))));
  CHECK(dpp::isnan(dpp::mul2(d32(dpp::nan))));
  CHECK(dpp::isnan(dpp::div10(d32(dpp::nan))));
  CHECK(dpp::isnan(dpp::div<3>(d32(dpp::nan))));
}

//////////////////////////////////////////////////////////////////////////////
// hashing across types
//////////////////////////////////////////////////////////////////////////////
void test_hashing_more()
{
  SECTION("hashing across types");

  std::hash<d16> h16;
  CHECK_EQ(h16(d16(123)), h16(d16(123)));
  CHECK_EQ(h16(d16(1.5)), h16(d16(1.5)));
  CHECK_EQ(h16(d16(direct, 15, -1)), h16(d16(direct, 150, -2)));
  CHECK_EQ(h16(d16(0)), h16(d16(direct, 0, 100)));

  std::hash<d128> h128;
  CHECK_EQ(h128(d128(1)), h128(d128(direct, 100, -2)));
  CHECK_EQ(h128(d128(direct, 12300, -2)), h128(d128(123)));
  CHECK_EQ(h128(d128(dpp::nan)), h128(d128(direct, 5, -2147483648LL)));

  std::unordered_set<d16> s;
  s.insert(d16(1));
  s.insert(d16(2));
  s.insert(d16(1));
  CHECK_EQ(s.size(), std::size_t(2));
  CHECK(s.count(d16(1)) == 1);
  CHECK(s.count(d16(3)) == 0);

  std::unordered_map<d64, int> m;
  m[d64(1)] = 1;
  m[d64(direct, 10, -1)] = 2;     // same value 1 -> overwrites
  CHECK_EQ(m.size(), std::size_t(1));
  CHECK_EQ(m.begin()->second, 2);
}

//////////////////////////////////////////////////////////////////////////////
// more identities
//////////////////////////////////////////////////////////////////////////////
void test_identities_more()
{
  SECTION("more identities");

  // binomial coefficients via the multiplicative formula stay integral
  {
    d64 c(1);
    for (int k = 1; k <= 20; ++k)
      c = c * d64(40 - k + 1) / d64(k);
    CHECK_EQ(c, d64(137846528820LL));
  }

  // Catalan numbers C_n = C_{n-1} * 2(2n-1) / (n+1)
  {
    d64 c(1);
    for (int n = 1; n <= 25; ++n)
      c = c * d64(2) * d64(2 * n - 1) / d64(n + 1);
    CHECK_EQ(c, d64(4861946401452LL));
  }

  // powers of three
  {
    d64 p(1);
    for (int i = 0; i < 37; ++i) p *= d64(3);
    CHECK_EQ(p, d64(450283905890997363LL));
  }

  // arithmetic series with its closed form
  {
    d64 s(0);
    for (int i = 1; i <= 1000000; ++i) s += d64(i);
    CHECK_EQ(s, d64(500000500000LL));
    CHECK_EQ(s, d64(1000000) * d64(1000001) / d64(2));
  }

  // sum of the first n odd numbers is n^2
  {
    for (int n : {10, 100, 1000})
    {
      d64 s(0);
      for (int k = 0; k < n; ++k) s += d64(2 * k + 1);
      CHECK_EQ(s, d64(n) * d64(n));
    }
  }

  // difference of squares
  {
    for (d64 a : {d64(3), d64(123), d64(99999)})
      for (d64 b : {d64(2), d64(100), d64(7777)})
        CHECK_EQ((a - b) * (a + b), a * a - b * b);
  }

  // telescoping sum: sum (1/k - 1/(k+1)) == 1 - 1/(n+1)
  {
    int const n = 50;
    d64 s(0);
    for (int k = 1; k <= n; ++k)
      s += d64(1) / d64(k) - d64(1) / d64(k + 1);
    d64 const expect = d64(1) - d64(1) / d64(n + 1);
    CHECK(dpp::abs(s - expect) < d64::eps * 100);
  }

  // geometric series with r = 3: sum == (r^(n+1) - 1) / (r - 1)
  {
    int const n = 30;
    d64 s(0), p(1);
    for (int i = 0; i <= n; ++i) { s += p; p *= d64(3); }
    CHECK_EQ(s, (p - d64(1)) / d64(2));
  }

  // (1/n) * n is within one ULP of 1 for the small types too
  {
    d16 const r16 = d16(1) / d16(3);
    CHECK(dpp::abs(d16(1) - r16 * d16(3)) <= d16::eps);

    d48 const r48 = d48(1) / d48(3);
    CHECK(dpp::abs(d48(1) - r48 * d48(3)) <= d48::eps);
  }
}

//////////////////////////////////////////////////////////////////////////////
// main
//////////////////////////////////////////////////////////////////////////////
int main()
{
  test_construction();
  test_arithmetic();
  test_comparisons();
  test_utilities();
  test_conversions();
  test_stream_io();
  test_special_values();
  test_literals();
  test_hashing();
  test_numeric();
  test_boundaries();
  test_precision();
  test_advanced();
  test_additional();
  test_significand_tie();

  test_type_traits();
  test_small_types();
  test_large_types();
  test_cross_type_promotions();
  test_native_interop();
  test_constexpr();
  test_ordering();
  test_rounding_and_alignment();
  test_exponent_extremes();
  test_to_decimal_more();
  test_to_string_more();
  test_stream_more();
  test_sqrt();
  test_elementary();
  test_rounding_semantics();
  test_increment_and_boolean();
  test_fused_midpoint_semantics();
  test_inv_semantics();
  test_binary_interop_semantics();
  test_quadratic_and_mandelbrot();
  test_factorial_exactness();
  test_to_integral();
  test_fast_div();
  test_hashing_more();
  test_identities_more();

  std::cerr << "\n"
    << g_checks   << " checks, "
    << g_failures << " failures, "
    << g_xfail_ok << " xfail-ok, "
    << g_xpass    << " xpass\n";

  return g_failures ? 1 : 0;
}
