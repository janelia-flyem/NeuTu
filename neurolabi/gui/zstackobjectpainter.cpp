#include "zstackobjectpainter.h"
#include "zpainter.h"
#include "geometry/zlinesegment.h"
#include "common/utilities.h"
#include "vis2d/zslicecanvas.h"
#include "data3d/displayconfig.h"

ZStackObjectPainter::ZStackObjectPainter()
{
}

void ZStackObjectPainter::paint(
    QPainter *painter, const ZStackObject *obj,
    const neutu::data3d::DisplayConfig &config)
{
  m_paintedHint = obj->display(painter, config);
}

void ZStackObjectPainter::Paint(
    ZSliceCanvas *canvas, const ZStackObject *obj,
    neutu::data3d::EDisplaySliceMode sliceMode,
    neutu::data3d::EDisplayStyle style)
{
  if (canvas) {
    ZSliceCanvasPaintHelper p(*canvas);
    QPainter *painter = p.getPainter();
    neutu::data3d::DisplayConfig config;
    config.setTransform(canvas->getTransform());
    config.setStyle(style);
    config.setSliceMode(sliceMode);
    canvas->setPainted(obj->display(painter, config));
  }
}

template<typename InputIterator>
void ZStackObjectPainter::Paint(
    ZSliceCanvas *canvas,
    const InputIterator &first, const InputIterator &last,
    neutu::data3d::EDisplaySliceMode sliceMode,
    neutu::data3d::EDisplayStyle style)
{
  neutu::data3d::DisplayConfig config;
  config.setTransform(canvas->getTransform());
  config.setStyle(style);
  config.setSliceMode(sliceMode);

  ZSliceCanvasPaintHelper p(*canvas);
  QPainter *painter = p.getPainter();

  for (InputIterator iter = first; iter != last; ++iter) {
    ZStackObject *obj = *iter;
    canvas->setPainted(obj->display(painter, config));
  }
}

template<typename InputIterator>
void ZStackObjectPainter::Paint(ZSliceCanvas *canvas,
    const InputIterator &first, const InputIterator &last,
    std::function<bool(const ZStackObject*)> pred,
    neutu::data3d::EDisplaySliceMode sliceMode,
    neutu::data3d::EDisplayStyle style)
{
  neutu::data3d::DisplayConfig config;
  config.setTransform(canvas->getTransform());
  config.setStyle(style);
  config.setSliceMode(sliceMode);

  ZSliceCanvasPaintHelper p(*canvas);
  QPainter *painter = p.getPainter();

  for (InputIterator iter = first; iter != last; ++iter) {
    ZStackObject *obj = *iter;
    if (pred(obj)) {
      canvas->setPainted(obj->display(painter, config));
    }
  }
}

void ZStackObjectPainter::Paint(
      ZSliceCanvas *canvas, const QList<ZStackObject*> &objList,
      std::function<bool(const ZStackObject*)> pred,
      neutu::data3d::EDisplaySliceMode sliceMode,
      neutu::data3d::EDisplayStyle style)
{
  Paint(canvas, objList.begin(), objList.end(), pred, sliceMode, style);
}

#if 0

void ZStackObjectPainter::setDisplayStyle(ZStackObject::EDisplayStyle style)
{
  m_style = style;
}

void ZStackObjectPainter::setSliceAxis(neutu::EAxis sliceAxis)
{
  m_axis = sliceAxis;
}

void ZStackObjectPainter::paint(
    const ZStackObject *obj, ZPainter &painter, int slice,
    ZStackObject::EDisplayStyle option, neutu::EAxis sliceAxis) const
{
  if (obj != NULL) {
    if (m_painterConst) {
      painter.save();
    }

    obj->display(painter, slice, option, sliceAxis);

    if (m_painterConst) {
      painter.restore();
    }
  }
}

void ZStackObjectPainter::paint(
    const std::shared_ptr<ZStackObject> &obj,
    ZPainter &painter, int slice, ZStackObject::EDisplayStyle option,
    neutu::EAxis sliceAxis) const
{
  paint(obj.get(), painter, slice, option, sliceAxis);
}

void ZStackObjectPainter::paint(
    const ZStackObject *obj, ZPainter &painter, int slice)
{
  paint(obj, painter, slice, m_style, m_axis);
}

void ZStackObjectPainter::paint(
    const ZLineSegment &seg, double width, const QColor &color,
    ZPainter &painter, int slice)
{
  double dataFocus = painter.getZ(slice);
  bool visible = false;
  ZLineSegment cross = GetFocusSegment(seg, visible, int(std::round(dataFocus)));
  if (visible) {
    if (m_painterConst) {
      painter.save();
    }

    QPen pen = painter.getPainter()->pen();
    pen.setWidthF(width);

    QColor lineColor = color;
    lineColor.setAlpha(128);
    pen.setColor(lineColor);

    painter.setPen(pen);

    painter.drawLine(
          QPointF(seg.getStartPoint().getX(), seg.getStartPoint().getY()),
          QPointF(seg.getEndPoint().getX(), seg.getEndPoint().getY()));

    lineColor.setAlpha(255);
    pen.setColor(lineColor);
    painter.setPen(pen);

    painter.drawLine(
          QPointF(cross.getStartPoint().getX(), cross.getStartPoint().getY()),
          QPointF(cross.getEndPoint().getX(), cross.getEndPoint().getY()));

    if (m_painterConst) {
      painter.restore();
    }
  }
}
#endif

#if 0
ZLineSegment ZStackObjectPainter::GetFocusSegment(
    const ZLineSegment &seg, bool &visible, int dataFocus)
{
  ZLineSegment result = seg;
  visible = false;

  double upperZ = dataFocus + 0.5;
  double lowerZ = dataFocus - 0.5;

  if (neutu::WithinOpenRange(seg.getStartPoint().getZ(), lowerZ, upperZ) &&
      neutu::WithinOpenRange(seg.getEndPoint().getZ(), lowerZ, upperZ)) {
    visible = true;
  } else {
    if (seg.getStartPoint().getZ() > seg.getEndPoint().getZ()) {
      result.flip();
    }

    if (result.getStartPoint().getZ() < upperZ &&
        result.getEndPoint().getZ() > lowerZ) {
      visible = true;
      double dz = result.getEndPoint().getZ() - result.getStartPoint().getZ();
      double lambda1 = (dataFocus - result.getStartPoint().getZ() - 0.5) / dz;
      double lambda2 = lambda1 + 1.0 / dz;
      if (lambda1 < 0.0) {
        lambda1 = 0.0;
      }
      if (lambda2 > 1.0) {
        lambda2 = 1.0;
      }

      result.set(result.getIntercept(lambda1), result.getIntercept(lambda2));
    }
  }

  return result;
}
#endif

