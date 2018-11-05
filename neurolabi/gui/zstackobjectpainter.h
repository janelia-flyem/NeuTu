#ifndef ZSTACKOBJECTPAINTER_H
#define ZSTACKOBJECTPAINTER_H

#include "zstackobject.h"

class ZPainter;

class ZStackObjectPainter
{
public:
  ZStackObjectPainter();

  void paint(
      const ZStackObject *obj,
      ZPainter &painter, int slice, ZStackObject::EDisplayStyle option,
      neutube::EAxis sliceAxis) const;

  void paint(const ZStackObject *obj, ZPainter &painter, int slice);

  void setRestoringPainter(bool on) {
    m_painterConst = on;
  }

  void setSliceAxis(neutube::EAxis sliceAxis);
  void setDisplayStyle(ZStackObject::EDisplayStyle style);

private:
  bool m_painterConst = true;
  ZStackObject::EDisplayStyle m_style = ZStackObject::NORMAL;
  neutube::EAxis m_axis = neutube::EAxis::Z;
};

#endif // ZSTACKOBJECTPAINTER_H
