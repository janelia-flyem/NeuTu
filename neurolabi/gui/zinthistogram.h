#ifndef ZINTHISTOGRAM_H
#define ZINTHISTOGRAM_H

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

  /*!
   * \brief Set histogram data.
   * \param hist C struct object.
   */
  void setData(int *hist);

  void clear();

  ZIntHistogram& operator= (const ZIntHistogram &hist);

  /*!
   * \brief Get the count of a certain value.
   * \param v The input value for retrieving the count.
   * \return The count of \a v.
   */
  int getCount(int v) const;

  /*!
   * \brief Get the mode of the histogram within a certain range.
   *
   * The function gets the mode (value at the peak) in [\a minV, \a maxV].
   *
   * \return The mode.
   */
  int getMode(int minV, int maxV) const;
  int getMode() const;

  /*!
   * \brief Get lower count
   * #{x | I(x) <= v}
   * \param v The threshold value.
   * \return lower count
   */
  int getLowerCount(int v) const;

  /*!
   * \brief Get upper count
   * #{x | I(x) >= v}
   * \param v The threshold value.
   * \return upper count
   */
  int getUpperCount(int v) const;

  /*!
   * \brief Get minimal value.
   * \return The minimal value that has non-zero count.
   */
  int getMinValue() const;

  /*!
   * \brief Get maximal value.
   * \return The maximal value that has non-zero count.
   */
  int getMaxValue() const;

  inline bool isEmpty() const {
    return m_hist == nullptr;
  }

  void print() const;

private:
  int *m_hist;
};

#endif // ZINTHISTOGRAM_H
