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
