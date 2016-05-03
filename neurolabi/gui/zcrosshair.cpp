#include "zcrosshair.h"
#include "zpainter.h"

ZCrossHair::ZCrossHair()
{
  init();
}

void ZCrossHair::init()
{
  setTarget(ZStackObject::TARGET_WIDGET);
  m_type = GetType();
  setZOrder(5);
  useCosmeticPen(true);
  setColor(QColor(255, 255, 255, 128));
}


void ZCrossHair::display(ZPainter &painter, int /*slice*/,
                         ZStackObject::EDisplayStyle /*style*/,
                         NeuTube::EAxis sliceAxis) const
{
  if (!isVisible() || painter.getCanvasRange().isEmpty()) {
    return;
  }

  ZPoint shiftedCenter = getCenter();
  shiftedCenter.shiftSliceAxis(sliceAxis);

  QPen pen(m_color, 2);
  pen.setStyle(Qt::SolidLine);
  pen.setCosmetic(m_usingCosmeticPen);

  painter.setPen(pen);

  QRectF canvasRange = painter.getCanvasRange();

  /*
  painter.drawLine(QPointF(canvasRange.left(), shiftedCenter.y()),
                   QPointF(canvasRange.right(), shiftedCenter.y()));
  painter.drawLine(QPointF(shiftedCenter.x(), canvasRange.top()),
                   QPointF(shiftedCenter.x(), canvasRange.bottom()));
                   */

  double gap = 10.0;
  painter.drawLine(QPointF(canvasRange.left(), shiftedCenter.y()),
                   QPointF(shiftedCenter.x() - gap, shiftedCenter.y()));
  painter.drawLine(QPointF(shiftedCenter.x() + gap, shiftedCenter.y()),
                   QPointF(canvasRange.right(), shiftedCenter.y()));

  painter.drawLine(QPointF(shiftedCenter.x(), canvasRange.top()),
                   QPointF(shiftedCenter.x(), shiftedCenter.y() - gap));
  painter.drawLine(QPointF(shiftedCenter.x(), shiftedCenter.y() + gap),
                   QPointF(shiftedCenter.x(), canvasRange.bottom()));
/*

  pen.setStyle(Qt::SolidLine);
//  pen.setColor(QColor(200, 200, 200, 255));
  painter.setPen(pen);
//  painter.drawPoint(QPointF(shiftedCenter.x(), shiftedCenter.y()));

  painter.drawLine(QPointF(shiftedCenter.x() - 0.2, shiftedCenter.y()),
                   QPointF(shiftedCenter.x() + 0.2, shiftedCenter.y()));
  painter.drawLine(QPointF(shiftedCenter.x(), shiftedCenter.y() - 0.2),
                   QPointF(shiftedCenter.x(), shiftedCenter.y() + 0.2));
                   */

}

void ZCrossHair::setCenter(double x, double y, double z)
{
  m_center.set(x, y, z);
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZCrossHair)
