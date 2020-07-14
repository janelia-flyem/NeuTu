#ifndef ZINTPOINTANNOTATIONCHUNK_H
#define ZINTPOINTANNOTATIONCHUNK_H

#include <limits>

#include "geometry/zintpoint.h"
#include "zitemchunk.hpp"

template<typename T>
class ZIntPointAnnotationChunk : public ZItemChunk<T, ZIntPoint>
{
public:
  ZIntPointAnnotationChunk() {
    this->m_invalidItem.invalidate();
  }

  /*!
   * \brief Pick an item within a range.
   *
   * When there are multiple items within the given range, whichever checked
   * first will be returned.
   */
  T pickItem(double x, double y, double z, double r);

  /*!
   * \brief Pick the closest item within a range.
   */
  T pickClosestItem(double x, double y, double z, double r);
  T hitClosestItem(double x, double y, double z, double r);

protected:
  ZIntPoint getKey(const T &item) const override
  {
    return getKey(item.getPosition());
  }

  ZIntPoint getKey(const ZIntPoint &pt) const
  {
    return pt;
  }

  ZIntPoint getKey(int x, int y, int z) const
  {
    return getKey(ZIntPoint(x, y, z));
  }
};

template<typename T>
T ZIntPointAnnotationChunk<T>::pickItem(double x, double y, double z, double r)
{
  for (auto item : this->m_itemMap) {
    T &t = item.second;
    double d2 = t.getPosition().distanceSquareTo(x, y, z);
    if (d2 <= r * r) {
      return t;
    }
  }

  return this->m_invalidItem;
}

template<typename T>
T ZIntPointAnnotationChunk<T>::pickClosestItem(
    double x, double y, double z, double r)
{
  T closestItem;

  double r2 = r * r;
  for (auto item : this->m_itemMap) {
    T &t = item.second;
    double minDist2 = std::numeric_limits<double>::infinity();
    double d2 = t.getPosition().distanceSquareTo(x, y, z);
//      double tr2 = t.getRadius() * t.getRadius();
    if (d2 <= r2 /*&& t.hit(x, y, z)*/) {
      if (d2 < minDist2) {
        minDist2 = d2;
        closestItem = t;
      }
    }
  }

  return closestItem;
}

template<typename T>
T ZIntPointAnnotationChunk<T>::hitClosestItem(
    double x, double y, double z, double r)
{
  T closestItem;

  double r2 = r * r;
  for (auto item : this->m_itemMap) {
    T &t = item.second;
    double minDist2 = std::numeric_limits<double>::infinity();
    double d2 = t.getPosition().distanceSquareTo(x, y, z);
//      double tr2 = t.getRadius() * t.getRadius();
    if (d2 <= r2 && t.hit(x, y, z)) {
      if (d2 < minDist2) {
        minDist2 = d2;
        closestItem = t;
      }
    }
  }

  return closestItem;
}

#endif // ZINTPOINTANNOTATIONCHUNK_H
