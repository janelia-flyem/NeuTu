#ifndef ZINTHISTOGRAM_H
#define ZINTHISTOGRAM_H

#include "tz_cdefs.h"

/*!
 * \brief The class of int histogram (integer value and integer count)
 *
 * It's a C++ wrapper of tz_int_histogram.h
 */
class ZIntHistogram
{
public:
  ZIntHistogram();
  ZIntHistogram(int *hist);
  ZIntHistogram(const ZIntHistogram &hist);
  ~ZIntHistogram();

  const int* c_hist() const { return m_hist; }
  int* c_hist() { return m_hist; }

  void setData(int *hist);

  void clear();

  ZIntHistogram& operator= (const ZIntHistogram &hist);

  int getCount(int v) const;
  int getMode(int minV, int maxV) const;
  int getMode() const;

  /*!
   * \brief Get lower count
   * #{x | I(x) <= v}
   * \param v
   * \return
   */
  int getLowerCount(int v) const;

  /*!
   * \brief Get upper count
   * #{x | I(x) >= v}
   * \param v
   * \return
   */
  int getUpperCount(int v) const;

  int getMinValue() const;
  int getMaxValue() const;

  inline bool isEmpty() const {
    return m_hist == NULL;
  }

  void print() const;

private:
  int *m_hist;
};

#endif // ZINTHISTOGRAM_H
