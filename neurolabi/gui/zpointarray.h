#ifndef ZPOINTARRAY_H
#define ZPOINTARRAY_H

#include <vector>
#include <zpoint.h>
#include <string>
#include "zcuboid.h"

class ZPointArray : public std::vector<ZPoint>
{
public:
  ZPointArray();

public:
  void translate(const ZPoint &pt);
  void scale(double sx, double sy, double sz);
  void exportSwcFile(const std::string &filePath, double radius) const;
  void exportTxtFile(const std::string &filePath) const;
  void print() const;

  /*!
   * \brief Append a point
   */
  void append(double x, double y, double z);
  void append(const ZPoint &pt);
  void append(const ZPointArray &ptArray);

  std::string toJsonString() const;

  ZPoint computeCenter() const;

  /*!
   * \brief Bounding box of the point array
   */
  ZCuboid getBoundBox() const;

  std::vector<double> computePlaneCov() const;

  bool isEmpty() const;
};

#endif // ZPOINTARRAY_H
