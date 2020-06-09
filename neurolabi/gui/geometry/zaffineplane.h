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
  ZAffinePlane(const ZPoint &offset, const ZPoint &v1, const ZPoint &v2);

  void set(const ZPoint &offset, const ZPoint &v1, const ZPoint &v2);
  void setPlane(const ZPoint &v1, const ZPoint &v2);
  void setOffset(const ZPoint &offset);
  void setOffset(double x, double y, double z);

  /*!
   * \brief Tranlate offset along a certain axis
   *
   * It sets \a v*normal when axis is neutu::EAxis::ARB.
   */
  void addOffset(double v, neutu::EAxis axis);

  ZPoint getV1() const;
  ZPoint getV2() const;
  ZPoint getOffset() const;

  ZPoint getNormal() const;

  ZPlane getPlane() const;

  bool onSamePlane(const ZAffinePlane &ap) const;
  bool isParallel(const ZAffinePlane &ap) const;

  bool contains(const ZPoint &pt) const;

  /*!
   * \brief Check if another plane lies within a band of the current plane.
   *
   * It returns true iff \a ap is parallel to the current plane and their
   * distance is within [-d, d).
   */
  bool contains(const ZAffinePlane &ap, double d) const;

  double computeSignedDistance(const ZPoint &pt) const;
  double computeSignedDistance(double x, double y, double z) const;

  void translate(double dx, double dy, double dz);
  void translate(const ZPoint &dv);
  void translateDepth(double d);
  void translateOnPlane(double du, double dv);

  /*!
   * \brief Align a point with the affine plane
   *
   * It aligns a point by tranforming it to the affine plane coordinate system,
   * of which the origin is the anchor point (offset).
   */
  ZPoint align(const ZPoint &pt) const;

  friend std::ostream& operator<<(std::ostream& stream, const ZAffinePlane &p);

  std::string toString() const;

  bool approxEquals(const ZAffinePlane &plane) const;

  bool operator== (const ZAffinePlane &p) const;
  bool operator!= (const ZAffinePlane &p) const;

private:
  ZPoint m_offset;
  ZPlane m_plane;
};

#endif // ZAFFINEPLANE_H
