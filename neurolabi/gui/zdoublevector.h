#ifndef ZDOUBLEVECTOR_H
#define ZDOUBLEVECTOR_H

#include <vector>
#include <string>
#include <initializer_list>

class ZDoubleVector : public std::vector<double>
{
public:
  ZDoubleVector();
  ZDoubleVector(size_t n);
  ZDoubleVector(size_t n, double val);
  ZDoubleVector(const double *data, std::size_t start,
                std::size_t end, std::size_t stride);
  ZDoubleVector(const std::vector<double> &array);
#ifndef SWIG
  ZDoubleVector(std::initializer_list<double> init);
#endif

public:
  inline double* dataArray() { return &((*this)[0]); }
  inline const double* dataArray() const { return &((*this)[0]); }
  double sum() const;
  double mean() const;
  double min(size_t *index = 0);
  double max(size_t *index = 0);
  double var() const;

  /*!
   * \brief Weighted sum of the indices
   */
  double indexSum() const;

  /*!
   * \brief Weighted mean of the indices
   */
  double indexMean() const;

  /*!
   * \brief Weighted sum of the quare of indices
   */
  double squaredIndexSum() const;

  /*!
   * \brief Weighted mean of the quare of indices
   */
  double squaredIndexMean() const;

  /*!
   * \brief Weighted variance of the indices
   */
  double indexVar() const;

  bool operator== (const ZDoubleVector &vec);

  /*!
   * \brief Load double array from a file
   *
   * Loads double array from a text file by reading as many as numbers from
   * the file. All values in the orignal buffer are cleared before the reading.
   */
  void importTextFile(const std::string &filePath);

  /*!
   * \brief Sort the double array.
   *
   * \param indexArray The variable to store the sorted element indices
   */
  void sort(std::vector<int> &indexArray);

  template <class T>
  static std::vector<std::vector<double> > reshape(std::vector<T> vec,
                                                   size_t innerSize);

  void exportDataFile(const std::string &filePath);
  static void exportTxtFile(const std::vector<std::vector<double> > &data,
                            const std::string &filePath);

  void print() const;
  static void Print(const std::vector<double> &vec);
  static void Print(const std::vector<std::vector<double> > &data);
};

template <class T>
std::vector<std::vector<double> > ZDoubleVector::reshape(std::vector<T> vec,
                                                         size_t innerSize)
{
  std::vector<std::vector<double> > array2d(vec.size() / innerSize);

  size_t vecIndex = 0;
  for (size_t i = 0; i < array2d.size(); ++i) {
    array2d[i].resize(innerSize);
    for (size_t j = 0; j < innerSize; ++j) {
      array2d[i][j] = vec[vecIndex++];
    }
  }

  return array2d;
}

#endif // ZDOUBLEVECTOR_H
