#ifndef ZAFFINERECT_H
#define ZAFFINERECT_H

#include "zaffineplane.h"
#include "zlinesegment.h"

/*!
 * \brief The class for defining a rectangle on an affine plane
 *
 * An affine rectangle is a rectangle on an affine plane, which is a plane with
 * an offset. Therefore, it has parameters of defining the plane as well as its
 * size, including its width and height. Either width or height can be 0,
 * indicating the rectangle is empty. But for some operations, a non-zero
 * dimension with zero for the other might make the rectangle like a line
 * segment. The offset of the rectangle is defined as the coordinates of its center,
 * which is located at half of the width and half of the height.
 *
 * Note: The name affine is from the affine space, which is the normal vector space
 * plus an offset. Don't confuse it with affine transformation.
 */
class ZAffineRect
{
public:
  ZAffineRect();

  void set(const ZPoint &offset, const ZPoint &v1, const ZPoint &v2,
           double width, double height);

  double getWidth() const;
  double getHeight() const;
  ZPoint getV1() const;
  ZPoint getV2() const;
  ZPoint getCenter() const;

  /*!
   * \brief Get a corner of the rectangle
   *
   * ^ Y
   * |
   * 1----0
   * |    |
   * |    |
   * 2----3 -> X
   */
  ZPoint getCorner(int index) const;
  ZPoint getMinCorner() const;
  ZPoint getMaxCorner() const;
  ZLineSegment getSide(int index) const;

  void setCenter(const ZPoint &offset);
  void setCenter(double x, double y, double z);
  void setPlane(const ZPoint &v1, const ZPoint &v2);
  void setPlane(const ZPlane &plane);
  void setPlane(const ZAffinePlane &plane);

  /*!
   * \brief Set the width and height of the rectangle
   *
   * Since negative size is not allowed, the width or height is set to 0 if
   * \a width or \a height is negative.
   */
  void setSize(double width, double height);

  /*!
   * \brief Set the size of the rectangle with one of its corners fixed.
   *
   * The rectangle is set to the size (\a width, \a height) with the corner indexed
   * by \a cornerIndex fixed at its position.
   *
   * Exceptional argument handling:
   *   Sets width or height to 0 if \a width or \a height is negative.
   *   Behaves the same as setSize(\a width, \a height) if \a cornerIndex is invalid.
   */
  void setSizeWithCornerFixed(double width, double height, int cornerIndex);

  void setSizeWithMinCornerFixed(double width, double height);
  void setSizeWithMaxCornerFixed(double width, double height);

  void translate(double dx, double dy, double dz);
  void translate(const ZPoint &dv);

  /*!
   * \brief translate on plane
   */
  void translateOnPlane(double du, double dv);
  void translateDepth(double d);

  /*!
   * \brief Scale a rectangle without moving its center
   */
  void scale(double su, double sv);

  inline const ZAffinePlane& getAffinePlane() const {
    return m_ap;
  }

  bool containsProjection(const ZPoint &pt) const;
  bool contains(const ZPoint &pt) const;
  bool contains(const ZAffineRect &rect) const;

  /*!
   * \brief Check a rectangle is with the band of another.
   *
   * It returns true if the affine plane of \a rect is within the plane band of
   * the object and all corners of \a rect is projected inside the current
   * rectangle.
   */
  bool contains(const ZAffineRect &rect, double d) const;

  bool contains(const ZPoint &pt, double d) const;

  bool isEmpty() const;
  bool isNonEmpty() const;

  friend std::ostream& operator<<(std::ostream& stream, const ZAffineRect &r);
  bool operator== (const ZAffineRect &p) const;
  bool operator!= (const ZAffineRect &p) const;

private:
  ZAffinePlane m_ap;
  double m_width = 0;
  double m_height = 0;
};

struct ZAffineRectBuilder
{
  ZAffineRectBuilder();
  ZAffineRectBuilder(int width, int height);
  operator ZAffineRect() const;

  ZAffineRectBuilder& at(const ZPoint &center);
  ZAffineRectBuilder& on(const ZPoint &v1, const ZPoint &v2);
  ZAffineRectBuilder& withSize(int width, int height);

private:
  ZAffineRect m_ar;
};

#endif // ZAFFINERECT_H
