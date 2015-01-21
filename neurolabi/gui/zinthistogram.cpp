#include "zinthistogram.h"
#include <iostream>
#include "tz_iarray.h"
#include "tz_int_histogram.h"

ZIntHistogram::ZIntHistogram() : m_hist(NULL)
{
}

ZIntHistogram::ZIntHistogram(int *hist) : m_hist(hist)
{

}

ZIntHistogram::ZIntHistogram(const ZIntHistogram &hist)
{
  const int *histArray = hist.c_hist();
  if (histArray == NULL) {
    m_hist = NULL;
  } else {
    m_hist = iarray_copy(const_cast<int*>(histArray), histArray[0] + 2);
  }
}


ZIntHistogram::~ZIntHistogram()
{
  clear();
}

void ZIntHistogram::clear()
{
  if (m_hist != NULL) {
    free(m_hist);
    m_hist = NULL;
  }
}

ZIntHistogram& ZIntHistogram::operator =(const ZIntHistogram &hist)
{
  const int *histArray = hist.c_hist();
  if (histArray == NULL) {
    m_hist = NULL;
  } else {
    clear();
    m_hist = iarray_copy(const_cast<int*>(histArray), histArray[0] + 2);
  }

  return *this;
}

int ZIntHistogram::getMaxValue() const
{
  return Int_Histogram_Max(m_hist);
}

int ZIntHistogram::getMinValue() const
{
  return Int_Histogram_Min(m_hist);
}

int ZIntHistogram::getCount(int v) const
{
  int count = 0;

  if (!isEmpty()) {
    if (v >= getMinValue() && v <= getMaxValue()) {
      count = m_hist[v + 2 - m_hist[1]];
    }
  }

  return count;
}

void ZIntHistogram::print() const
{
  if (isEmpty()) {
    std::cout << "Empty histogram" << std::endl;
  } else {
    Print_Int_Histogram(m_hist);
  }
}
