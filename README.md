# dpp
This is a an easily understandable **decimal** floating-point library for non-critical tasks and testing. The library does not adhere to any particular standard, but was inspired by [DEC64](https://github.com/douglascrockford/DEC64).

The underlying design goal was to extract as many **decimal** digits as possible out of the **`int32_t`**/**`int64_t`** pair, with an eye towards the **`int64_t`**/**`int128_t`** pair. Hopefully at tolerable performance and binary size.
# build instructions
    g++ -std=c++2a -Ofast -s mandelbrot.cpp -o m
    em++ -std=c++20 -O3 -s eps.cpp -o e.js
# alternatives
* [Boost.Multiprecision](https://github.com/boostorg/multiprecision) - artisanal c++, highly recommended,
* [DEC64](https://github.com/douglascrockford/DEC64) - legendary asm, one 64-bit type,
* [decimal_for_cpp](https://github.com/vpiotr/decimal_for_cpp) - c++, fixed-point, bloated (locales),
* [libdecnumber](https://github.com/gcc-mirror/gcc/tree/master/libdecnumber) - epic c, supported by gcc, complex, huge.
