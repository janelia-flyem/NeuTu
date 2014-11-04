#ifndef ZCLOSEDCURVE_H
#define ZCLOSEDCURVE_H

#include "zpointarray.h"

class ZJsonObject;
class ZLineSegment;
class ZCuboid;

class ZClosedCurve
{
public:
  ZClosedCurve();

  bool isEmpty() const;

  size_t getLandmarkNumber() const;

  /*!
   * \brief Get a landmark.
   *
   * The index is periodic.
   */
  const ZPoint& getLandmark(int index) const;

  /*!
   * \brief Get curve length.
   *
   * Note that a closed curve with only two landmarks has two overlapped
   * segments.
   */
  double getLength() const;

  /*!
   * \brief Get a resampled curve with fix number of landmarks.
   */
  ZClosedCurve resampleF(int n) const;

  //ZClosedCurve resample(double step) const;

  /*!
   * \brief Index shift to match a given curve
   *
   * The return value s means that the sth element of the curve matches the
   * first element of \a target if it is non-negative. If it is negative, the
   * first element of the curve matchs the -sth element of \a target.
   */
  int findMatchShift(const ZClosedCurve &target) const;

  size_t getSegmentNumber() const;

  /*!
   * \brief Get line segment.
   *
   * The index is periodic. It returns (0, 0) -> (0, 0) if the curve is empty.
   */
  ZLineSegment getSegment(size_t index) const;

  /*!
   * \brief Append a landmark to the last.
   */
  void append(const ZPoint &pt);
  void append(double x, double y, double z);

  ZClosedCurve interpolate(const ZClosedCurve &curve, double lambda, int shift);
  ZClosedCurve* interpolate(
      const ZClosedCurve &curve, double lambda, int shift, ZClosedCurve *result);

  void reverse();

  size_t getMinXIndex() const;
  size_t getMinYIndex() const;

  void print() const;

  void loadJsonObject(const ZJsonObject &obj);
  ZJsonObject toJsonObject() const;

  void clear();

  ZPoint computeCenter() const;
  ZPoint computeDirection() const;

  ZCuboid getBoundBox() const;

  void scale(double sx, double sy, double sz);
  void translate(double dx, double dy);

  ZClosedCurve* clone() const;

private:
  ZPointArray m_landmarkArray;
  static const ZPoint m_emptyPlaceHolder;
};

#endif // ZCLOSEDCURVE_H
