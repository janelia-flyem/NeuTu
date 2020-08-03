#ifndef ZPLANE_H
#define ZPLANE_H

#include "zpoint.h"

/*!
 * \brief The class for repsenting a plane.
 *
 * The plane is defined by two orthonormal vectors, v1 and v2, on the plane.
 * The normal of the plane is v1 x v2.
 */
class ZPlane
{
public:
  ZPlane();
  ZPlane(const ZPoint &v1, const ZPoint &v2);

  bool isValid() const;
  void invalidate();

  inline const ZPoint& getV1() const {
    return m_v1;
  }

  inline const ZPoint& getV2() const {
    return m_v2;
  }

  /*!
   * \brief Set a plane by giving two vectors of the plane.
   *
   * The vectors will be ajusted if they are not orthonormal:
   *  1. if \a v1 and \a v2 are origins, the plane will be set to (1,0,0)x(0,1,0)
   *  2. if one of them is origin and the other is not, or \a v1 and \a v2 are
   *     parallel, then the non-zero vector will be considered as a rotation from
   *     (1, 0, 0) for \a v1 or (0, 1, 0) for \a v2, therefore the result plane
   *     is the same rotation from (1,0,0)x(0,1,0).
   *  3. if \a v1 and \a v2 defines a plane, then \a v2 will be ajdusted to be
   *     perpendicular to \a v2. Both will be normailzed.
   */
  void set(const ZPoint &v1, const ZPoint &v2);

  ZPoint getNormal() const;
  bool onSamePlane(const ZPlane &p) const;
  bool contains(const ZPoint &pt) const;
  double computeSignedDistance(const ZPoint &pt) const;
  double computeSignedDistance(double x, double y, double z) const;

  /*!
   * \brief Align a point with plane
   *
   * It aligns a point by tranforming it to the plane coordinate system.
   *
   * \return Transformed point.
   */
  ZPoint align(const ZPoint &pt) const;

  ZPoint mapAligned(double u, double v) const;

  friend std::ostream& operator<<(std::ostream& stream, const ZPlane &p);

  bool approxEquals(const ZPlane &plane) const;
  bool operator== (const ZPlane &p) const;
  bool operator!= (const ZPlane &p) const;

private:
  static bool IsValid(const ZPoint &v1, const ZPoint &v2);

private:
  ZPoint m_v1 = ZPoint(1, 0, 0);
  ZPoint m_v2 = ZPoint(0, 1, 0);
};

#endif // ZPLANE_H
