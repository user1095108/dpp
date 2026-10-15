#include "../dpp.hpp"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
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
using dpp::d32;
using dpp::d64;
using dpp::d128;
using dpp::d256;
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

  std::cerr << "\n"
    << g_checks   << " checks, "
    << g_failures << " failures, "
    << g_xfail_ok << " xfail-ok, "
    << g_xpass    << " xpass\n";

  return g_failures ? 1 : 0;
}
