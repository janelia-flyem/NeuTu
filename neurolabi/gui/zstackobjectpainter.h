#ifndef ZSTACKOBJECTPAINTER_H
#define ZSTACKOBJECTPAINTER_H

#include <memory>

#include "zstackobject.h"

class ZPainter;
class ZLineSegment;

class ZStackObjectPainter
{
public:
  ZStackObjectPainter();

  void paint(
      const ZStackObject *obj,
      ZPainter &painter, int slice, ZStackObject::EDisplayStyle option,
      neutu::EAxis sliceAxis) const;
  void paint(
      const std::shared_ptr<ZStackObject> &obj,
      ZPainter &painter, int slice, ZStackObject::EDisplayStyle option,
      neutu::EAxis sliceAxis) const;

  void paint(const ZStackObject *obj, ZPainter &painter, int slice);
  void paint(
      const ZLineSegment &seg, double width, const QColor &color,
      ZPainter &painter, int slice);

  void setRestoringPainter(bool on) {
    m_painterConst = on;
  }

  void setSliceAxis(neutu::EAxis sliceAxis);
  void setDisplayStyle(ZStackObject::EDisplayStyle style);

  static ZLineSegment GetFocusSegment(
      const ZLineSegment &seg, bool &visible, int dataFocus);

private:
  bool m_painterConst = true;
  ZStackObject::EDisplayStyle m_style = ZStackObject::EDisplayStyle::NORMAL;
  neutu::EAxis m_axis = neutu::EAxis::Z;
};

#endif // ZSTACKOBJECTPAINTER_H
