![screenshot.png](screenshot.png?raw=true)
# dpp
This is an intuitive arbitrary-precision **decimal** floating-point number library for non-critical tasks and testing. The library does not adhere to any standard, but was inspired by [DEC64](https://github.com/douglascrockford/DEC64).
# build instructions
    git submodule update --init
    g++ -std=c++2a -Ofast -s mandelbrot.cpp -o m
    em++ -std=c++20 -O3 -s eps.cpp -o e.js
# mathematical functions
`dpp` is compatible with the [C++ Standard Library](https://en.wikipedia.org/wiki/C%2B%2B_Standard_Library) and you can use the standard math library implementations (e.g. `std::cos(0_d32)`, ...), or, preferrably, write your own.
# resources
* [Boost.Multiprecision](https://github.com/boostorg/multiprecision)
* [DEC64](https://github.com/douglascrockford/DEC64)
* [General Decimal Arithmetic](https://speleotrove.com/decimal)
* [decimal_for_cpp](https://github.com/vpiotr/decimal_for_cpp)
* [libdecnumber](https://github.com/gcc-mirror/gcc/tree/master/libdecnumber)
