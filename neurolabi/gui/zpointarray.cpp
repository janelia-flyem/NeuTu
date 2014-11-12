#include "zpointarray.h"
#include <fstream>
#include <iostream>
#include "zjsonobject.h"
#include "zjsonparser.h"
#include "zstring.h"
#include "zgraph.h"

ZPointArray::ZPointArray()
{
}

void ZPointArray::translate(const ZPoint &pt)
{
  for (iterator iter = begin(); iter != end(); ++iter) {
    *iter += pt;
  }
}

void ZPointArray::exportSwcFile(const std::string &filePath, double radius) const
{
  std::ofstream stream(filePath.c_str());

  for (size_t i = 0; i < size(); ++i) {
    int parentId = -1;
    const ZPoint &pt = (*this)[i];
    stream << i + 1 << " 2 " << pt.x() << " " << pt.y() << " " << pt.z()
           << " " << radius << " " << parentId << std::endl;
  }

  stream.close();
}

void ZPointArray::exportTxtFile(const std::string &filePath) const
{
    std::ofstream stream(filePath.c_str());

    for (size_t i = 0; i < size(); ++i) {
        const ZPoint &pt = (*this)[i];
        stream << pt.x() << " " << pt.y() << " " << pt.z() << std::endl;
    }

    stream.close();
}

void ZPointArray::print() const
{
  for (size_t i = 0; i < size(); ++i) {
    const ZPoint &pt = (*this)[i];
    std::cout << pt.toString() << std::endl;
  }
}

void ZPointArray::scale(double sx, double sy, double sz)
{
  if (sx != 1.0 || sy != 1.0 || sz != 1.0) {
    for (iterator iter = begin(); iter != end(); ++iter) {
      ZPoint &pt = *iter;
      pt.setX(pt.x() * sx);
      pt.setY(pt.y() * sy);
      pt.setZ(pt.z() * sz);
    }
  }
}

void ZPointArray::append(double x, double y, double z)
{
  push_back(ZPoint(x, y, z));
}

void ZPointArray::append(const ZPoint &pt)
{
  push_back(pt);
}

void ZPointArray::append(const ZPointArray &ptArray)
{
  insert(end(), ptArray.begin(), ptArray.end());
}

std::string ZPointArray::toJsonString() const
{
  std::string str;

  if (!empty()) {
    ZJsonObject obj(json_object(), true);

    //json_t *rootObj = obj.getValue();
    json_t *pointListObj = json_array();
    obj.consumeEntry("point-list", pointListObj);
    //json_object_set_new(rootObj, "point-list", pointListObj);
    for (ZPointArray::const_iterator iter = begin(); iter != end(); ++iter) {
      const ZPoint &pt = *iter;

      ZJsonArray pointObj;
      pointObj << pt.x() << pt.y() << pt.z();
      json_array_append(pointListObj, pointObj.getValue());
      /*
      json_t *pointObj = json_array();
      json_array_append_new(pointObj, json_real(pt.x()));
      json_array_append_new(pointObj, json_real(pt.y()));
      json_array_append_new(pointObj, json_real(pt.z()));

      json_array_append_new(pointListObj, pointObj);
      */
    }

    str = obj.dumpString();
  }

  return str;
}

ZCuboid ZPointArray::getBoundBox() const
{
  ZCuboid boundBox;
  if (!empty()) {
    const ZPoint &pt = (*this)[0];
    boundBox.set(pt.x(), pt.y(), pt.z(), pt.x(), pt.y(), pt.z());
  }

  for (ZPointArray::const_iterator iter = begin(); iter != end(); ++iter) {
    boundBox.include(*iter);
  }

  return boundBox;
}

ZPoint ZPointArray::computeCenter() const
{
  ZPoint center(0.0, 0.0, 0.0);
  if (!empty()) {
    for (const_iterator iter = begin(); iter != end(); ++iter) {
      center += *iter;
    }

    center /= size();
  }

  return center;
}

bool ZPointArray::isEmpty() const
{
  return empty();
}

std::vector<double> ZPointArray::computePlaneCov() const
{
  std::vector<double> cov(3, 0.0);

  if (!isEmpty()) {
    double xMean = 0.0;
    double yMean = 0.0;
    double xSquareMean = 0.0;
    double ySquareMean = 0.0;
    double xyCorr = 0.0;

    size_t count = 0;
    for (ZPointArray::const_iterator iter = begin(); iter != end(); ++iter) {
      const ZPoint &pt = *iter;
      xMean += pt.x();
      xSquareMean += pt.x() * pt.x();
      yMean += pt.y();
      ySquareMean += pt.y() * pt.y();
      xyCorr += pt.x() * pt.y();
      ++count;
    }

    xMean /= count;
    yMean /= count;
    xSquareMean /= count;
    ySquareMean /= count;
    xyCorr /= count;

#ifdef _DEBUG_2
    std::cout << count << std::endl;
#endif

    double factor = 1.0;
    if (count > 1) {
      factor = static_cast<double>(count) / (count - 1);
    }

    cov[0] = (xSquareMean - xMean * xMean) * factor;
    cov[1] = (ySquareMean - yMean * yMean) * factor;
    cov[2] = (xyCorr - xMean * yMean) * factor;
  }

  return cov;
}

void ZPointArray::importTxtFile(const std::string &filePath)
{
  clear();
  ZString str;
  FILE *fp = fopen(filePath.c_str(), "r");
  if (fp != NULL) {
    while (str.readLine(fp)) {
      std::vector<int> coord = str.toIntegerArray();
      if (coord.size() == 3) {
        append(coord[0], coord[1], coord[2]);
      }
    }
  }
  fclose(fp);
}

void ZPointArray::importPcdFile(const std::string &filePath)
{
  clear();
  ZString str;
  FILE *fp = fopen(filePath.c_str(), "r");
  if (fp != NULL) {
    while (str.readLine(fp)) {
      str.trim();
      if (str.empty()) {
        continue;
      }

      if (str.startsWith("#")) {
        continue;
      }

      if (std::isdigit(str[0]) || str[0] == '-' || str[0] == '.') {
        std::vector<double> coord = str.toDoubleArray();
        if (coord.size() == 3) {
          append(coord[0], coord[1], coord[2]);
        }
      }
    }
  }
  fclose(fp);
}

ZGraph* ZPointArray::computeDistanceGraph(double maxDist) const
{
  ZGraph *graph = new ZGraph(ZGraph::UNDIRECTED_WITH_WEIGHT);
  for (size_t i = 0; i < size(); ++i) {
    const ZPoint &pt1 = (*this)[i];
    for (size_t j = i + 1; j < size(); ++j) {
      const ZPoint &pt2 = (*this)[j];
      double d = pt1.distanceTo(pt2);
      if (d <= maxDist) {
        graph->addEdgeFast(i, j, d);
      }
    }
  }
  return graph;
}
