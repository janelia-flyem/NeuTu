#include "zdoublevector.h"

#include <fstream>
#include <iostream>

#include "tz_darray.h"
#include "zstring.h"

using namespace std;

ZDoubleVector::ZDoubleVector()
{
}

ZDoubleVector::ZDoubleVector(size_t n) : std::vector<double>(n)
{
}

ZDoubleVector::ZDoubleVector(size_t n, double val) : std::vector<double>(n, val)
{
}

ZDoubleVector::ZDoubleVector(const double *data, size_t start, size_t end,
                             size_t stride)
{
  resize((end - start) / stride + 1);

  for (size_t i = 0; i < size(); i++) {
    (*this)[i] = *data;
    data += stride;
  }
}

ZDoubleVector::ZDoubleVector(const std::vector<double> &array)
{
  resize(array.size());
  copy(array.begin(), array.end(), begin());
}

double ZDoubleVector::mean() const
{
  return darray_mean(dataArray(), size());
}

double ZDoubleVector::sum() const
{
  return darray_sum(dataArray(), size());
}

double ZDoubleVector::var() const
{
  return darray_var(dataArray(), size());
}

double ZDoubleVector::max(size_t *index)
{
  return darray_max(dataArray(), size(), index);
}

double ZDoubleVector::min(size_t *index)
{
  return darray_min(dataArray(), size(), index);
}

double ZDoubleVector::indexSum() const
{
  double s = 0.0;
  size_t index = 0;
  for (const_iterator iter = begin(); iter != end(); ++iter, ++index) {
    s += *iter * index;
  }

  return s;
}

double ZDoubleVector::indexMean() const
{
  double s = sum();

  if (s == 0.0) {
    return 0.0;
  }

  return indexSum() / sum();
}

double ZDoubleVector::squaredIndexSum() const
{
  double s = 0.0;
  size_t index = 0;
  for (const_iterator iter = begin(); iter != end(); ++iter, ++index) {
    s += *iter * index * index;
  }

  return s;
}

double ZDoubleVector::squaredIndexMean() const
{
  double s = sum();

  if (s == 0.0) {
    return 0.0;
  }

  return squaredIndexSum() / sum();
}

double ZDoubleVector::indexVar() const
{
  double mu = indexMean();

  return squaredIndexMean() - mu * mu;
}

bool ZDoubleVector::operator ==(const ZDoubleVector &vec)
{
  if (size() != vec.size()) {
    return false;
  }

  for (size_t i = 0; i < size(); i++) {
    if ((*this)[i] != vec[i]) {
      return false;
    }
  }

  return true;
}

void ZDoubleVector::exportDataFile(const std::string &filePath)
{
  darray_write(filePath.c_str(), dataArray(), size());
}

void ZDoubleVector::exportTxtFile(
    const vector<vector<double> > &data, const std::string &filePath)
{
  std::ofstream stream(filePath.c_str());

  for (size_t i = 0; i < data.size(); ++i) {
    for (size_t j = 0; j < data[i].size(); j++) {
      stream << data[i][j] << " ";
    }
    stream << std::endl;
  }

  stream.close();
}

void ZDoubleVector::print(const std::vector<std::vector<double> > &data)
{
  for (size_t i = 0; i < data.size(); ++i) {
    print(data[i]);
  }
}

void ZDoubleVector::print() const
{
  print(*this);
}

void ZDoubleVector::print(const std::vector<double> &vec)
{
  for (size_t j = 0; j < vec.size(); j++) {
    cout << vec[j] << " ";
  }
  cout << endl;
}

void ZDoubleVector::importTextFile(const string &filePath)
{
  clear();
  FILE *fp = fopen(filePath.c_str(), "r");

  if (fp != NULL) {
    ZString str;
    while (str.readLine(fp)) {
      std::vector<double> array = str.toDoubleArray();
      insert(end(), array.begin(), array.end());
    }
  }
}

void ZDoubleVector::sort(std::vector<int> &indexArray)
{
  indexArray.resize(size());

  if (!empty()) {
    darray_qsort(&((*this)[0]), &(indexArray[0]), size());
  }
}
