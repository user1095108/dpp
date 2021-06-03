// https://github.com/vpiotr/decimal_for_cpp/issues/34
#include <iostream>

#include "dpp.hpp"

template <typename T>
void computeMullersIter(int iteration)
{
    T xi = 4, xii = 4.25;
    for (int i = 0; i < iteration; i++)
    {
        T x = 108 - ((815 - 1500/xi) / xii);
        std::cout << "Iter:" << i << " : x=" << x << std::endl;
        xi = xii;
        xii = x;
    }
}



int main()
{
    std::cout << "float" << std::endl;
    computeMullersIter<float>(30);
    std::cout << "dpp::d32" << std::endl;
    computeMullersIter<dpp::d32>(30);
    std::cout << "double" << std::endl;
    computeMullersIter<double>(30);
    std::cout << "dpp::d64" << std::endl;
    computeMullersIter<dpp::d64>(30);
}
