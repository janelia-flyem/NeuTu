#include "zmatrix.h"
#if defined(_QT_GUI_USED_)
#include <QDebug>
#endif
#include <ostream>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "tz_error.h"
#include "tz_darray.h"

using namespace std;

ZMatrix::ZMatrix() : m_rowNumber(0), m_columnNumber(0)
{
}

ZMatrix::ZMatrix(int rowNumber, int columnNumber)
{
  m_rowNumber = rowNumber;
  m_columnNumber = columnNumber;
  int count = rowNumber * columnNumber;
  m_data.resize(count);
  /*
  m_data = (double**) calloc(rowNumber, sizeof(double*));
  for (int i = 0; i < rowNumber; i++) {
    m_data[i] = (double*) calloc(columnNumber, sizeof(double));
  }
  */
}

ZMatrix::~ZMatrix()
{
  clear();
}

void ZMatrix::resize(int rowNumber, int columnNumber)
{
  TZ_ASSERT(rowNumber >= 0, "Invalid row number");
  TZ_ASSERT(columnNumber >= 0, "Invalid column number");

  if (rowNumber == getRowNumber() && columnNumber == getColumnNumber()) {
    return;
  }

  int count = rowNumber * columnNumber;
  m_data.resize(count, 0);
  if (columnNumber <= getColumnNumber()) {
    if (columnNumber < getColumnNumber()) {
      //Move data
      int copyRowNumber = imin2(rowNumber, getRowNumber());
      size_t srcOffset = 0;
      size_t dstOffset = 0;
      int dc = getColumnNumber() - columnNumber;

      //For each row
      for (int i = 0; i < copyRowNumber; ++i) {
        //Assign a row of values
        for (int j = 0; j < columnNumber; ++j) {
          m_data[dstOffset++] = m_data[srcOffset++];
        }
        srcOffset += dc;
      }
    }
  } else {
    ZDoubleVector data(count, 0);
    int copyRowNumber = imin2(rowNumber, getRowNumber());
    size_t srcOffset = 0;
    size_t dstOffset = 0;
    int dc = columnNumber - getColumnNumber();

    //For each row
    for (int i = 0; i < copyRowNumber; ++i) {
      //Assign a row of values
      for (int j = 0; j < getColumnNumber(); ++j) {
        data[dstOffset++] = m_data[srcOffset++];
      }
      dstOffset += dc;
    }
    m_data = data;
  }

  m_rowNumber = rowNumber;
  m_columnNumber = columnNumber;

#if 0
  //Free unwanted rows
  if (rowNumber < getRowNumber()) {
    for (int i = rowNumber; i < getRowNumber(); ++i) {
      free(m_data[i]);
      m_data[i] = NULL;
    }
    m_rowNumber = rowNumber;
  }

  //Realloc rows
  if (rowNumber != getRowNumber()) {
    m_data = (double**) realloc(m_data, rowNumber * sizeof(double*));
    if (rowNumber > getRowNumber()) {
      for (int i = getRowNumber(); i < rowNumber; ++i) {
        m_data[i] = (double*) calloc(columnNumber, sizeof(double));
      }
    }
  }

  //Reset columns
  if (columnNumber != getColumnNumber()) {
    for (int i = 0; i < getRowNumber(); ++i) {
      m_data[i] = (double*) realloc(m_data[i], columnNumber * sizeof(double));
    }
  }

  m_rowNumber = rowNumber;
  m_columnNumber = columnNumber;
#endif
}

void ZMatrix::setConstant(double value)
{
  size_t count = getElementNumber();
  for (size_t i = 0; i < count; ++i) {
    m_data[i] = value;
  }
}

void ZMatrix::set(int index, double value)
{
  if (index < 0 || index >= getSize()) {
    return;
  }

  m_data[index] = value;
}

double ZMatrix::getValue(int index) const
{
  if (index < 0 || index >= getSize()) {
    return 0.0;
  }

  return m_data[index];
}

int ZMatrix::sub2index(int row, int col) const
{
  int index = -1;
  if ((row >= 0) && (row < m_rowNumber) &&
      (col >= 0) && (col < m_columnNumber)) {
    index = row * m_columnNumber + col;
  }

  return index;
}

pair<int, int> ZMatrix::index2sub(int index) const
{
  pair<int, int> sub;
  sub.first = index / m_columnNumber;
  sub.second = index % m_columnNumber;

  return sub;
}

void ZMatrix::debugOutput()
{
  for (int i = 0; i < m_rowNumber; i++) {
    for (int j = 0; j < m_columnNumber; j++) {
      cout << at(i, j) << " ";
    }
    cout << endl;
  }
  /*
  ostringstream stream;

  stream << m_rowNumber << " x " << m_columnNumber << " matrix\n";
  for (int i = 0; i < m_rowNumber; i++) {
    if (i > 10) {
      stream << "...\n";
      break;
    }
    for (int j = 0; j < m_columnNumber; j++) {
      if (j > 10) {
        stream << " ... ";
        break;
      }
      stream << m_data[2][4] << " ";
    }
    stream << '\n';
  }

  cout << stream.str().c_str();
  */
}

double* ZMatrix::rowPointer(int row)
{
  return const_cast<double*>(
        (static_cast<const ZMatrix&>(*this)).rowPointer(row));
}

const double* ZMatrix::rowPointer(int row) const
{
  if (row > getRowNumber()) {
    return NULL;
  }

  return &(m_data[row * getColumnNumber()]);
}



void ZMatrix::copyValueFrom(double *data)
{
  memcpy(rowPointer(0), data, getElementNumber() * sizeof(double));
#if 0
  for (int i = 0; i < getRowNumber(); ++i) {
    memcpy(m_data[i], data + getColumnNumber() * i,
           getColumnNumber() * sizeof(double));
  }
#endif

#ifdef _DEBUG_2
  cout << "copied:" << endl;
  for (int i = 0; i < getRowNumber(); i++) {
    for (int j = 0; j < getColumnNumber(); j++) {
      cout << getValue(i, j) << " ";
    }
    cout << endl;
  }
  cout << endl;
#endif
}

void ZMatrix::copyColumnValueFrom(double *data, int columnStart, int columnNumber)
{
  if (columnNumber + columnStart > getColumnNumber()) {
    resize(getRowNumber(), columnNumber + columnStart);
  }

  for (int i = 0; i < getRowNumber(); ++i) {
    memcpy(rowPointer(i) + columnStart, data + columnNumber * i,
           columnNumber * sizeof(double));
  }
}

bool ZMatrix::exportCsv(const string &path)
{
  std::ofstream stream(path.c_str());

  if (!stream.is_open()) {
    return false;
  }

  for (int i = 0; i < m_rowNumber; i++) {
    for (int j = 0; j < m_columnNumber; j++) {
      stream << at(i, j);
      if (j != m_columnNumber - 1) {
        stream << ",";
      }
    }
    stream << endl;
  }

  stream.close();

  return true;
}

bool ZMatrix::exportCsv(
    const string &path, const std::vector<string> &rowName,
    const std::vector<string> &columnName)
{
  if (isEmpty()) {
    return false;
  }

  if (getRowNumber() != (int) rowName.size() ||
      getColumnNumber() != (int) columnName.size()) {
    return false;
  }

  std::ofstream stream(path.c_str());

  if (!stream.is_open()) {
    return false;
  }

  if (getColumnNumber() == (int) columnName.size()) {
    if (getRowNumber() == (int) rowName.size()) {
      stream << "name,";
    }

    for (int j = 0; j < getColumnNumber(); j++) {
      stream << columnName[j];
      if (j != getColumnNumber() - 1) {
        stream << ",";
      }
    }
    stream << endl;
  }

  for (int i = 0; i < m_rowNumber; i++) {
    if (getRowNumber() == (int) rowName.size()) {
      stream << rowName[i] << ",";
    }
    for (int j = 0; j < m_columnNumber; j++) {
      stream << at(i, j);
      if (j != m_columnNumber - 1) {
        stream << ",";
      }
    }
    stream << endl;
  }

  stream.close();

  return true;
}

double ZMatrix::getRowMax(int row, int *index) const
{
  size_t arrayIndex = 0;

  double v = darray_max(rowPointer(row), getColumnNumber(), &arrayIndex);

  if (index != NULL) {
    *index = (int) arrayIndex;
  }

  return v;
}

double ZMatrix::getColumnMax(int column, int *index) const
{
  if (index != NULL) {
    *index = 0;
  }
  double v = 0.0;

  if (getRowNumber() > 0) {
    v = getValue(0, column);
    for (int i = 1; i < getRowNumber(); ++i) {
      double tv = getValue(i, column);
      if (tv > v) {
        v = tv;
        if (index != NULL) {
          *index = i;
        }
      }
    }
  }

  return v;
}


void ZMatrix::clear()
{
  m_data.clear();

  m_rowNumber = 0;
  m_columnNumber = 0;
}

void ZMatrix::importTextFile(const string &filePath)
{
  clear();

  double *value = darray_load_matrix(filePath.c_str(), NULL, &m_columnNumber,
                                     &m_rowNumber);
  m_data.resize(m_rowNumber * m_columnNumber);

//  m_data = (double**) calloc(m_rowNumber, sizeof(double*));
//  for (int i = 0; i < m_rowNumber; i++) {
//    m_data[i] = (double*) calloc(m_columnNumber, sizeof(double));
//  }

  copyValueFrom(value);

  free(value);
}

int ZMatrix::copyRowValueTo(
    int row, int columnStart, int columnEnd, double *dst) const
{
  if (dst == NULL) {
    return 0;
  }

  if (row >= getRowNumber()) {
    return 0;
  }

  if (columnStart > columnEnd || columnStart >= getColumnNumber()) {
    return 0;
  }

  if (columnEnd < 0) {
    return 0;
  }

  if (columnStart < 0) {
    dst -= columnStart;
    columnStart = 0;
  }

  if (columnEnd >= getColumnNumber()) {
    columnEnd = getColumnNumber() - 1;
  }

  int length = columnEnd - columnStart + 1;
  memcpy(dst, rowPointer(row) + columnStart, sizeof(double) * length);

  return length;
}

bool ZMatrix::setRowValue(int row, const std::vector<double> &rowValue)
{
  if (row < 0 || row >= getRowNumber()) {
    return false;
  }

  if (rowValue.empty() || (int) rowValue.size() != getColumnNumber()) {
    return false;
  }

  memcpy(rowPointer(row), &(rowValue[0]), sizeof(double) * rowValue.size());

  return true;
}

bool ZMatrix::setRowValue(
    int row, int columnStart, const std::vector<double> &rowValue)
{
  if (row < 0 || row >= getRowNumber()) {
    return false;
  }

  if (columnStart < 0) {
    return false;
  }

  if (rowValue.empty() ||
      (int) rowValue.size() != getColumnNumber() - columnStart) {
    return false;
  }

  memcpy(rowPointer(row) + columnStart, &(rowValue[0]),
      sizeof(double) * rowValue.size());

  return true;
}

bool ZMatrix::isEmpty() const
{
  return (m_rowNumber == 0) || (m_columnNumber == 0);
}

void ZMatrix::printInfo() const
{
  std::cout << m_rowNumber << " x " << m_columnNumber << " matrix" << std::endl;
}

ZMatrix ZMatrix::makeRowSlice(int r0, int r1) const
{
  ZMatrix mat(r1 - r0 + 1, getColumnNumber());
  for (int i = r0; i <= r1; ++i) {
    copyRowValueTo(i, 0, getColumnNumber() - 1, mat.rowPointer(i - r0));
  }

  return mat;
}

ZMatrix ZMatrix::makeColumnSlice(int c0, int c1) const
{
  ZMatrix mat(getRowNumber(), c1 - c0 + 1);
  for (int i = 0; i < getRowNumber(); ++i) {
    copyRowValueTo(i, c0, c1, mat.rowPointer(i));
  }

  return mat;
}

ZDoubleVector ZMatrix::getDiag() const
{
  ZDoubleVector vec(imin2(getRowNumber(), getColumnNumber()));
  for (size_t i = 0; i < vec.size(); ++i) {
    vec[i] = getValue(i, i);
  }

  return vec;
}

void ZMatrix::setDiag(double v)
{
  int n = imin2(getRowNumber(), getColumnNumber());
  for (int i = 0; i < n; ++i) {
    set(i, i, v);
  }
}
