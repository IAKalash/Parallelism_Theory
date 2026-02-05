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
    TYPE sum = 0;

    for (size_t i = 0; i < n; i++) {
        arr[i] = sin(i * step);
        sum += arr[i];
    }

    std::cout << sum << std::endl;

    return 0;
}