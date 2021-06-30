# dpp
This is a an easily understandable **decimal** floating-point library library for non-critical tasks and testing. The library does not adhere to any particular standard, but was inspired by [DEC64](https://github.com/douglascrockford/DEC64).

Rather than use a library that is:
* unsupported (e.g. on ARM),
* hard to integrate (e.g. `DEC64`),
* hard to serialize,
* bloated,
* non-portable (e.g. `DEC64`).

I am using this header-only library, that gets the job done, even though it is probably not a marvel of numerical programming.
# build instructions
    g++ -std=c++2a -Ofast -s mandelbrot.cpp -o m
    em++ -std=c++20 -O3 -s eps.cpp -o e.js
If you have problems installing the latest `gcc` and `clang`, have a look [here](https://serverfault.com/questions/22414/how-can-i-run-debian-stable-but-install-some-packages-from-testing).
# alternatives
* [Boost.Multiprecision](https://github.com/boostorg/multiprecision) - artisanal c++, highly recommended,
* [DEC64](https://github.com/douglascrockford/DEC64) - legendary asm, one 64-bit type,
* [decimal_for_cpp](https://github.com/vpiotr/decimal_for_cpp) - c++, fixed-point, bloated (locales),
* [libdecnumber](https://github.com/gcc-mirror/gcc/tree/master/libdecnumber) - epic c, supported by gcc, complex, huge.
