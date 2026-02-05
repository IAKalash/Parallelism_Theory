#include <iostream>
#include <vector>
#include <math.h>

#ifndef TYPE
#define TYPE double
#endif

int main() {
    size_t n = 10000000;
    std::vector <TYPE> arr(n);

    const double step = M_PI * 2 / n;

    for (size_t i = 0; i < n; i++) {
        arr[i] = sin(i * step);
    }

    TYPE sum = 0;
    for (auto a : arr) sum += a;
    std::cout << sum << std::endl;

    return 0;
}