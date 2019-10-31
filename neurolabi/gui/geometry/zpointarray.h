#ifndef ZPOINTARRAY_H
#define ZPOINTARRAY_H

#include <vector>
#include <string>
#include <limits>

#include "zpoint.h"
#include "zcuboid.h"


class ZGraph;

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

  void importTxtFile(const std::string &filePath);
  void importPcdFile(const std::string &filePath);

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

  ZGraph *computeDistanceGraph(
      double maxDist = std::numeric_limits<double>::infinity()) const;
};

#endif // ZPOINTARRAY_H
