#ifndef ZCROSSHAIR_H
#define ZCROSSHAIR_H

#include "zstackobject.h"
#include "zpoint.h"

class ZCrossHair : public ZStackObject
{
public:
  ZCrossHair();

  static ZStackObject::EType GetType() {
    return ZStackObject::TYPE_CROSS_HAIR;
  }

  virtual const std::string& className() const;

public:
  virtual void display(ZPainter &painter, int slice,
                       EDisplayStyle option, NeuTube::EAxis sliceAxis) const;

  ZPoint getCenter() const {
    return m_center;
  }

  void setCenter(double x, double y, double z);

private:
  void init();

private:
  ZPoint m_center;
};

#endif // ZCROSSHAIR_H
