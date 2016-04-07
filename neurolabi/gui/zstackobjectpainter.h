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
      NeuTube::EAxis sliceAxis) const;

  void setRestoringPainter(bool on) {
    m_painterConst = on;
  }

private:
  void init();

private:
  bool m_painterConst;
};

#endif // ZSTACKOBJECTPAINTER_H
