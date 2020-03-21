#ifndef ZAFFINERECT_H
#define ZAFFINERECT_H

#include "zaffineplane.h"
#include "zlinesegment.h"

class ZAffineRect
{
public:
  ZAffineRect();

  void set(const ZPoint &offset, const ZPoint &v1, const ZPoint &v2,
           int width, int height);

  int getWidth() const;
  int getHeight() const;
  ZPoint getV1() const;
  ZPoint getV2() const;
  ZPoint getCenter() const;

  ZPoint getCorner(int index) const;
  ZLineSegment getSide(int index) const;

  void setCenter(const ZPoint &offset);
  void setPlane(const ZPoint &v1, const ZPoint &v2);
  void setSize(int width, int height);

  ZAffinePlane getAffinePlane() const;

  friend std::ostream& operator<<(std::ostream& stream, const ZAffineRect &r);

private:
  ZAffinePlane m_ap;
  int m_width = 0;
  int m_height = 0;
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
