#ifndef ZARBSLICEVIEWPARAM_H
#define ZARBSLICEVIEWPARAM_H

#include "geometry/zintpoint.h"
#include "geometry/zpoint.h"

class QRect;
class ZAffinePlane;
class ZAffineRect;

/*!
 * \brief The class of managing parameters of viewing an arbitrarily cut slice.
 */
class ZArbSliceViewParam
{
public:
  ZArbSliceViewParam();

  void setCenter(const ZIntPoint &center);
  void setCenter(int x, int y, int z);
  void setPlane(const ZPoint &v1, const ZPoint &v2);
  void setSize(int width, int height);

  /*!
   * \brief Same as setSize for API compatiblity with ZStackViewParam
   */
  void resize(int width, int height);

  QRect getViewPort() const;
  int getX() const;
  int getY() const;
  int getZ() const;
  ZIntPoint getCenter() const;

  int getX0() const;
  int getY0() const;

  ZPoint getPlaneV1() const;
  ZPoint getPlaneV2() const;
  ZPoint getPlaneNormal() const;
  int getWidth() const;
  int getHeight() const;

  ZAffinePlane getAffinePlane() const;
  ZAffineRect getAffineRect() const;

  /*!
   * \brief Move in the slicing coordinate system.
   *
   * (\a dx, \a dy, \a dz) are the moving steps along each slicing axis.
   * Nothing will be done if the parameter is invalid.
   */
  void move(double dx, double dy, double dz);

  bool contains(const ZArbSliceViewParam &param);

  bool operator ==(const ZArbSliceViewParam &param) const;
  bool operator !=(const ZArbSliceViewParam &param) const;

  bool hasSamePlaneCenter(const ZArbSliceViewParam &param) const;

  bool isValid() const;

  /*!
   * \brief Check if the affine planes overlap and have the same orientation.
   */
  bool isSamePlane(const ZArbSliceViewParam &param) const;

private:
  int m_width = 0;
  int m_height = 0;
  ZIntPoint m_center;
  ZPoint m_v1;
  ZPoint m_v2;
};

#endif // ZARBSLICEVIEWPARAM_H
