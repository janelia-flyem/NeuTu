#ifndef NUMERICARRAY_H
#define NUMERICARRAY_H

#include <cstddef>

namespace neutu {

namespace array {

double Var(const double *x, size_t n);
void Sort(double *x, int *idx, int n);
double Max(const double* d1, size_t length, size_t* idx);
double* LoadMatrix(const char *filepath, double *d, int *m, int *n);
double* MedianFilter(const double *in, size_t length, int wndsize, double *out);
double* AverageSmooth(
    const double* in, size_t length, int wndsize, double *out);

int* Negate(int* d1, size_t length);
int* IntMalloc(size_t n);
}

}

#endif // NUMERICARRAY_H
