![screenshot.png](screenshot.png?raw=true)
# dpp
This is an easily understandable decimal floating-point library for non-critical tasks and testing. The library does not adhere to any standard, but was inspired by [DEC64](https://github.com/douglascrockford/DEC64).

The underlying design goal was to maximize the number of extractable decimal digits of the **`(int32_t, int64_t)`** pair, with an eye towards the **`(int64_t, int128_t)`** pair, at tolerable performance and binary sizes.
# build instructions
    g++ -std=c++2a -Ofast -s mandelbrot.cpp -o m
    em++ -std=c++20 -O3 -s eps.cpp -o e.js
# alternatives
* [Boost.Multiprecision](https://github.com/boostorg/multiprecision)
* [DEC64](https://github.com/douglascrockford/DEC64)
* [decimal_for_cpp](https://github.com/vpiotr/decimal_for_cpp)
* [libdecnumber](https://github.com/gcc-mirror/gcc/tree/master/libdecnumber)
