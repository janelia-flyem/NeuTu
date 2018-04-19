#include "zpositionmapper.h"

#include "zviewproj.h"
#include "zpoint.h"
#include "geometry/zaffineplane.h"

ZPositionMapper::ZPositionMapper()
{

}

QPointF ZPositionMapper::WidgetToStack(double x, double y, const ZViewProj &vp)
{
  return WidgetToStack(QPointF(x, y), vp);
}

QPointF ZPositionMapper::WidgetToStack(const QPointF &pt, const ZViewProj &vp)
{
  return vp.mapPointBackF(pt);
}

QPointF ZPositionMapper::WidgetToRawStack(double x, double y, const ZViewProj &vp)
{
  return WidgetToRawStack(QPointF(x, y), vp);
}

QPointF ZPositionMapper::WidgetToRawStack(const QPointF &pt, const ZViewProj &vp)
{
  QPointF newPt = WidgetToStack(pt, vp);
  newPt.setX(newPt.x() - vp.getCanvasRect().left());
  newPt.setY(newPt.y() - vp.getCanvasRect().right());

  return newPt;
}

ZPoint ZPositionMapper::WidgetToStack(
    double x, double y, double z, const ZViewProj &vp, double z0)
{
  QPointF pt = WidgetToStack(x, y, vp);
  ZPoint newPos(pt.x(), pt.y(), z + z0);

  return newPos;
}

ZPoint ZPositionMapper::WidgetToStack(
    double x, double y, const ZViewProj &vp, double z0)
{
  return WidgetToStack(x, y, 0, vp, z0);
}

ZPoint ZPositionMapper::StackToData(
    const ZPoint &pt, int xvc, int yvc, const ZAffinePlane &ap)
{

}
