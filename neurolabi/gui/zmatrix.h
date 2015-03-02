#ifndef ZMATRIX_H
#define ZMATRIX_H

#include <vector>
#include <string>

#include "zdoublevector.h"

/**
 * @brief The ZMatrix class
 *
 * Note that each row is stored in one array.
 */
class ZMatrix
{
public:
  ZMatrix();
  ZMatrix(int rowNumber, int columnNumber);
  virtual ~ZMatrix();

  /*!
   * \brief Clear matrix buffer.
   *
   * Row number and column number are set to 0.
   */
  void clear();

  /*!
   * \brief Test if the matrix has any element
   *
   * \return true iff there is no element in the matrix
   */
  bool isEmpty() const;

  void setConstant(double value);
  inline void set(int i, int j, double value) {
    m_data[getColumnNumber() * i + j]= value; }

  /*!
   * \brief Set the value of the element at \a index
   *
   * nothing is done if index is out of range.
   */
  void set(int index, double value);

  inline double getValue(int i, int j) const {
    return at(i, j); }

  inline void addValue(int i, int j, double dv) {
    m_data[getColumnNumber() * i + j] += dv; }

  /*!
   * \brief Get the value by index.
   *
   * \return 0.0 if \a index is out of range.
   */
  double getValue(int index) const;

  inline double& at(int i, int j) { return m_data[getColumnNumber() * i + j]; }
  inline const double& at(int i, int j) const {
    return m_data[getColumnNumber() * i + j]; }
  inline int getSize() const { return m_rowNumber * m_columnNumber; }

  void resize(int rowNumber, int columnNumber);

  inline int getRowNumber() const { return m_rowNumber; }
  inline int getColumnNumber() const { return m_columnNumber; }
  inline size_t getElementNumber() const {
    return getRowNumber() * getColumnNumber(); }

  int sub2index(int row, int col) const;
  std::pair<int, int> index2sub(int index) const;

  void debugOutput();

  double *rowPointer(int row);
  const double *rowPointer(int row) const;

  void copyValueFrom(double *data);

  /*!
   * \brief Copy a column to an array.
   */
  void copyColumnValueFrom(double *data, int columnStart, int columnNumber);

  /*!
   * \brief Copy a row to an array
   *
   * Copy the the \a columnStart - \a columnEnd elements at row \a row to
   * \a data. It only copies value within the valie range. Any element out of
   * range is treated as void, which does nothing on \a data but shifting its
   * pointer position. The user has to make sure \a data is sufficiently
   * allocated.
   *
   * \a return Actual number of values copied.
   */
  int copyRowValueTo(int row, int columnStart, int columnEnd, double *dst) const;

  /*!
   * \brief Set the value of a certain row
   *
   * Set the row at \a row by the values in \a rowValue, which must have the
   * same size as the row number.
   *
   * \return true iff the row is set successfully.
   */
  bool setRowValue(int row, const std::vector<double> &rowValue);

  /*!
   * \brief Set the value of a certain row
   *
   * \param row The target row
   * \param columnStart Starting position of the row. No negative value is allowed.
   * \param rowValue Row values. The size must match the number of columns to fill.
   * \return true if the row is set successfully.
   */
  bool setRowValue(int row, int columnStart,
                   const std::vector<double> &rowValue);

  ZDoubleVector getDiag() const;
  void setDiag(double v);

#if 0
  /*!
   * \brief Append a row to the end of the matrix
   *
   * \param The row of values, which must have the same size as the row number.
   *
   * \return true iff the row is appended successfully.
   */
  bool appendRow(const std::vector<double> rowValue);
#endif

  /*!
   * \brief Load double array from a file
   *
   * Loads matrix from a text file. All values in the orignal buffer are cleared
   * before the reading.
   */
  void importTextFile(const std::string &filePath);

  /*!
   * \brief Export the matrix into a CSV file
   * \param Output file path
   * \return true iff the matrix is exported successfully.
   */
  bool exportCsv(const std::string &path);
  bool exportCsv(const std::string &path,
                 const std::vector<std::string> &rowName,
                 const std::vector<std::string> &columnName);

  void printInfo() const;

public:
  ZMatrix makeRowSlice(int r0, int r1) const;
  ZMatrix makeColumnSlice(int c0, int c1) const;

public: //Feature matrix functions
  std::vector<int> kmeans(int k);
  std::vector<int> weightedKmeans(int k);

  /*!
   * \brief Get the maximum value of a row
   * \param The specified value.
   * \return The maximum of value of row \row. The column index is stored in
   *         \a index if \a index is not NULL.
   */
  double getRowMax(int row, int *index = NULL) const;

  double getColumnMax(int column, int *index = NULL) const;

private:
  ZDoubleVector m_data;
  int m_rowNumber;
  int m_columnNumber;
};


#endif // ZMATRIX_H
