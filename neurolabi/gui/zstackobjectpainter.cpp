#include "zstackobjectpainter.h"
#include "zpainter.h"
#include "geometry/zlinesegment.h"
#include "tz_utilities.h"

ZStackObjectPainter::ZStackObjectPainter()
{
}

void ZStackObjectPainter::setDisplayStyle(ZStackObject::EDisplayStyle style)
{
  m_style = style;
}

void ZStackObjectPainter::setSliceAxis(neutube::EAxis sliceAxis)
{
  m_axis = sliceAxis;
}

void ZStackObjectPainter::paint(
    const ZStackObject *obj, ZPainter &painter, int slice,
    ZStackObject::EDisplayStyle option, neutube::EAxis sliceAxis) const
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
    const ZStackObject *obj, ZPainter &painter, int slice)
{
  paint(obj, painter, slice, m_style, m_axis);
}

ZLineSegment ZStackObjectPainter::GetFocusSegment(
    const ZLineSegment &seg, bool &visible, int dataFocus)
{
  ZLineSegment result = seg;
  visible = false;

  double upperZ = dataFocus + 0.5;
  double lowerZ = dataFocus - 0.5;

  if (IS_IN_OPEN_RANGE(seg.getStartPoint().getZ(), lowerZ, upperZ) &&
      IS_IN_OPEN_RANGE(seg.getEndPoint().getZ(), lowerZ, upperZ)) {
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

void ZStackObjectPainter::paint(
    const ZLineSegment &seg, double width, const QColor &color,
    ZPainter &painter, int slice)
{
  double dataFocus = painter.getZ(slice);
  bool visible = false;
  ZLineSegment cross = GetFocusSegment(seg, visible, dataFocus);
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
