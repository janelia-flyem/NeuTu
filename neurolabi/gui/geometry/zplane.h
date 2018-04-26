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

  ZPoint getV1() const;
  ZPoint getV2() const;
  ZPoint getNormal() const;

  void set(const ZPoint &v1, const ZPoint &v2);


private:
  ZPoint m_v1 = ZPoint(1, 0, 0);
  ZPoint m_v2 = ZPoint(0, 1, 0);
};

#endif // ZPLANE_H
