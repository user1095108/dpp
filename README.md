![screenshot.png](screenshot.png?raw=true)
# dpp
This is an intuitive **decimal** floating-point number library for non-critical tasks and testing. The library does not adhere to any standard, but was inspired by [DEC64](https://github.com/douglascrockford/DEC64).
# build instructions
    git submodule update --init
    g++ -std=c++23 -Ofast -s mandelbrot.cpp -o m
    em++ -std=c++23 -O3 -s eps.cpp -o e.js
# resources
* [Boost.Multiprecision](https://github.com/boostorg/multiprecision)
* [DEC64](https://github.com/douglascrockford/DEC64)
* [General Decimal Arithmetic](https://speleotrove.com/decimal)
* [decimal_for_cpp](https://github.com/vpiotr/decimal_for_cpp)
* [libdecnumber](https://github.com/gcc-mirror/gcc/tree/master/libdecnumber)
