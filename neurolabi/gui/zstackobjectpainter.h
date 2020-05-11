#ifndef ZSTACKOBJECTPAINTER_H
#define ZSTACKOBJECTPAINTER_H

#include <memory>
#include <functional>

#include <QList>

#include "zstackobject.h"

class ZSliceCanvas;

//class ZPainter;
//class ZLineSegment;

class ZStackObjectPainter
{
public:
  ZStackObjectPainter();

  void paint(QPainter *painter, const ZStackObject *obj,
             const neutu::data3d::DisplayConfig &config);

  bool isPainted() const {
    return m_paintedHint;
  }

  static void Paint(
      ZSliceCanvas *canvas, const ZStackObject *obj,
      neutu::data3d::EDisplaySliceMode sliceMode,
      neutu::data3d::EDisplayStyle style);

  static void Paint(
      ZSliceCanvas *canvas, const QList<ZStackObject*> &objList,
      std::function<bool(const ZStackObject*)> pred,
      neutu::data3d::EDisplaySliceMode sliceMode,
      neutu::data3d::EDisplayStyle style);

private:
  template<typename InputIterator>
  static void Paint(
      ZSliceCanvas *canvas,
      const InputIterator &first, const InputIterator &last,
      neutu::data3d::EDisplaySliceMode sliceMode,
      neutu::data3d::EDisplayStyle style);

  template<typename InputIterator>
  static void Paint(
      ZSliceCanvas *canvas,
      const InputIterator &first, const InputIterator &last,
      std::function<bool(const ZStackObject*)> pred,
      neutu::data3d::EDisplaySliceMode sliceMode,
      neutu::data3d::EDisplayStyle style);


  /*
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
      */

  /*
  void setRestoringPainter(bool on) {
    m_painterConst = on;
  }

  void setSliceAxis(neutu::EAxis sliceAxis);
  void setDisplayStyle(ZStackObject::EDisplayStyle style);

  static ZLineSegment GetFocusSegment(
      const ZLineSegment &seg, bool &visible, int dataFocus);
      */

private:
  bool m_paintedHint = false;
  /*
  bool m_painterConst = true;
  ZStackObject::EDisplayStyle m_style = ZStackObject::EDisplayStyle::NORMAL;
  neutu::EAxis m_axis = neutu::EAxis::Z;
  */
};

#endif // ZSTACKOBJECTPAINTER_H
