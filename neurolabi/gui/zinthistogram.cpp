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

void ZIntHistogram::setData(int *hist)
{
  clear();
  m_hist = hist;
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

int ZIntHistogram::getLowerCount(int v) const
{
  int count = 0;
  if (!isEmpty()) {
    int minV = getMinValue();
    int maxV = std::min(v, getMaxValue());
    for (int i = minV; i <= maxV; ++i) {
      count += getCount(i);
    }
  }

  return count;
}

int ZIntHistogram::getUpperCount(int v) const
{
  int count = 0;
  if (!isEmpty()) {
    int minV = std::max(v, getMinValue());
    int maxV = getMaxValue();
    for (int i = minV; i <= maxV; ++i) {
      count += getCount(i);
    }
  }

  return count;
}

int ZIntHistogram::getMode(int minV, int maxV) const
{
  minV = std::max(minV, getMinValue());
  maxV = std::min(maxV, getMaxValue());

  int m = minV;

  int maxCount = getCount(m);
  for (int i = minV; i <= maxV; ++i) {
    int c = getCount(i);
    if (c > maxCount) {
      maxCount = c;
      m = i;
    }
  }

  return m;
}

int ZIntHistogram::getMode() const
{
  return getMode(getMinValue(), getMaxValue());
}

void ZIntHistogram::print() const
{
  if (isEmpty()) {
    std::cout << "Empty histogram" << std::endl;
  } else {
    Print_Int_Histogram(m_hist);
  }
}
