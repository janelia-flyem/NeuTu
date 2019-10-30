#include "numericarray.h"

#include "tz_darray.h"
#include "tz_iarray.h"

double neutu::array::Var(const double *x, size_t n)
{
  return darray_var(x, n);
}

void neutu::array::Sort(double *x, int *idx, int n)
{
  darray_qsort(x, idx, n);
}

double neutu::array::Max(const double* d1, size_t length, size_t* idx)
{
  return darray_max(d1, length, idx);
}

double* neutu::array::LoadMatrix(const char *filepath, double *d, int *m, int *n)
{
  return darray_load_matrix(filepath, d, m, n);
}

double* neutu::array::MedianFilter(
    const double *in, size_t length, int wndsize, double *out)
{
  return darray_medfilter(in, length, wndsize, out);
}

double* neutu::array::AverageSmooth(
    const double* in, size_t length, int wndsize, double *out)
{
  return darray_avgsmooth(in, length, wndsize, out);
}

int* neutu::array::Negate(int* d1, size_t length)
{
  return iarray_neg(d1, length);
}

int* neutu::array::IntMalloc(size_t n)
{
  return iarray_malloc(n);
}
