#ifndef ZARBSLICEVIEWPARAM_H
#define ZARBSLICEVIEWPARAM_H

#include "zintpoint.h"
#include "zpoint.h"

class QRect;

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

  QRect getViewPort() const;
  int getX() const;
  int getY() const;
  int getZ() const;
  ZIntPoint getCenter() const;
  ZPoint getPlaneV1() const;
  ZPoint getPlaneV2() const;
  int getWidth() const;
  int getHeight() const;


  bool contains(const ZArbSliceViewParam &param);

  bool isValid() const;

private:
  int m_width = 0;
  int m_height = 0;
  ZIntPoint m_center;
  ZPoint m_v1;
  ZPoint m_v2;
};

#endif // ZARBSLICEVIEWPARAM_H
