#ifndef NUMERICARRAY_H
#define NUMERICARRAY_H

#include <cstddef>

namespace neutu {

namespace array {

double Var(const double *x, size_t n);
void Sort(double *x, int *idx, int n);
double Max(const double* x, size_t n, size_t* idx);
double* LoadMatrix(const char *filepath, double *d, int *m, int *n);
double* MedianFilter(const double *in, size_t length, int wndsize, double *out);
double* AverageSmooth(
    const double* in, size_t length, int wndsize, double *out);

int Max(const int* x, size_t n, size_t* idx);
int* Negate(int* x, size_t n);
int* IntMalloc(size_t n);
int* IntCalloc(size_t n);
int* Clone(const int* x, size_t n);
}

}

#endif // NUMERICARRAY_H
