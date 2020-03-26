#ifndef ZAFFINEPLANE_H
#define ZAFFINEPLANE_H

#include "zplane.h"

/*!
 * \brief The affine plane class
 *
 * The affine plane is plane that has an anchor point, which defines the origin
 * of the affine space.
 */
class ZAffinePlane
{
public:
  ZAffinePlane();

  void set(const ZPoint &offset, const ZPoint &v1, const ZPoint &v2);
  void setPlane(const ZPoint &v1, const ZPoint &v2);
  void setOffset(const ZPoint &offset);

  ZPoint getV1() const;
  ZPoint getV2() const;
  ZPoint getOffset() const;

  ZPoint getNormal() const;

  ZPlane getPlane() const;

  bool onSamePlane(const ZAffinePlane &ap) const;
  bool contains(const ZPoint &pt) const;
  double computeSignedDistance(const ZPoint &pt) const;
  double computeSignedDistance(double x, double y, double z) const;

  /*!
   * \brief Align a point with the affine plane
   *
   * It aligns a point by tranforming it to the affine plane coordinate system,
   * of which the origin is the anchor point (offset).
   */
  ZPoint align(const ZPoint &pt) const;

  friend std::ostream& operator<<(std::ostream& stream, const ZAffinePlane &p);

  std::string toString() const;

private:
  ZPoint m_offset;
  ZPlane m_plane;
};

#endif // ZAFFINEPLANE_H
