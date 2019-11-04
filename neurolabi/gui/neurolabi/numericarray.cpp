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

double neutu::array::Max(const double* x, size_t n, size_t* idx)
{
  return darray_max(x, n, idx);
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

int* neutu::array::Negate(int* x, size_t n)
{
  return iarray_neg(x, n);
}

int* neutu::array::IntMalloc(size_t n)
{
  return iarray_malloc(n);
}

int* neutu::array::IntCalloc(size_t n)
{
  return iarray_calloc(n);
}

int* neutu::array::Clone(const int* x, size_t n)
{
  return iarray_copy(const_cast<int*>(x), n);
}

int neutu::array::Max(const int* x, size_t n, size_t* idx)
{
  return iarray_max(x, n, idx);
}
