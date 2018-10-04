#include "zpositionmapper.h"

#include "zviewproj.h"
#include "zpoint.h"
#include "zintpoint.h"
#include "geometry/zaffineplane.h"
#include "geometry/zgeometry.h"

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
  newPt.setY(newPt.y() - vp.getCanvasRect().top());

  return newPt;
}

ZPoint ZPositionMapper::WidgetToRawStack(
      const ZIntPoint &pt, const ZViewProj &viewProj)
{
  QPointF mapped2d = WidgetToRawStack(QPointF(pt.getX(), pt.getY()), viewProj);

  return ZPoint(mapped2d.x(), mapped2d.y(), pt.getZ());
}

ZPoint ZPositionMapper::WidgetToRawStack(
      const ZPoint &pt, const ZViewProj &viewProj)
{
  QPointF mapped2d = WidgetToRawStack(QPointF(pt.getX(), pt.getY()), viewProj);

  return ZPoint(mapped2d.x(), mapped2d.y(), pt.getZ());
}

ZPoint ZPositionMapper::WidgetToStack(
    double x, double y, double z, const ZViewProj &vp, double z0)
{
  QPointF pt = WidgetToStack(x, y, vp);
  ZPoint newPos(pt.x(), pt.y(), z + z0);

  return newPos;
}

ZPoint ZPositionMapper::WidgetToStack(
    const ZPoint &pt, const ZViewProj &vp, double z0)
{
  return WidgetToStack(pt.getX(), pt.getY(), pt.getZ(), vp, z0);
}

ZPoint ZPositionMapper::WidgetToStack(
    double x, double y, const ZViewProj &vp, double z0)
{
  return WidgetToStack(x, y, 0, vp, z0);
}

ZPoint ZPositionMapper::WidgetToStack(
    const ZIntPoint &pt, const ZViewProj &viewProj, double z0)
{
  return WidgetToStack(pt.toPoint(), viewProj, z0);
}

ZPoint ZPositionMapper::StackToData(
    const ZPoint &pt, const ZPoint &stackAnchor, const ZAffinePlane &ap)
{
  ZPoint dp = pt - stackAnchor;
  dp = ap.getV1() * dp.getX() + ap.getV2() * dp.getY() +
       ap.getNormal() * dp.getZ() + ap.getOffset();

  return dp;
}

ZPoint ZPositionMapper::StackToData(double x, double y, const ZAffinePlane &ap)
{
  double dx = x - ap.getOffset().getX();
  double dy = y - ap.getOffset().getY();
  ZPoint dp = ap.getV1() * dx + ap.getV2() * dy + ap.getOffset();

  return dp;
}


ZPoint ZPositionMapper::StackToData(const QPointF &pt, const ZAffinePlane &ap)
{
  return StackToData(pt.x(), pt.y(), ap);
}

ZPoint ZPositionMapper::StackToData(const ZPoint &pt, const ZAffinePlane &ap)
{
  return StackToData(pt, ap.getOffset(), ap);
}

ZPoint ZPositionMapper::StackToData(const ZPoint &pt, neutube::EAxis axis)
{
  ZPoint pt2 = pt;
  pt2.shiftSliceAxisInverse(axis);

  return pt2;
}


ZPoint ZPositionMapper::DataToStack(
    const ZPoint &pt, const ZPoint &stackAnchor, const ZAffinePlane &ap)
{
  ZPoint dp = pt - ap.getOffset();

  ZPoint apNormal = ap.getNormal();
  ZPoint v1(ap.getV1().getX(), ap.getV2().getX(), apNormal.getX());
  ZPoint v2(ap.getV1().getY(), ap.getV2().getY(), apNormal.getY());
  ZPoint v3(ap.getV1().getZ(), ap.getV2().getZ(), apNormal.getZ());

  dp = v1 * dp.getX() + v2 * dp.getY() + v3 * dp.getZ() + stackAnchor;

  return dp;
}


ZPoint ZPositionMapper::DataToStack(const ZPoint &pt, const ZAffinePlane &ap)
{
  return DataToStack(pt, ap.getOffset(), ap);
}

ZPoint ZPositionMapper::DataToStack(const ZPoint &pt, neutube::EAxis axis)
{
  ZPoint pt2 = pt;
  pt2.shiftSliceAxis(axis);

  return pt2;
}

ZPoint ZPositionMapper::mapDataToStack(const ZPoint &pt)
{
  ZPoint result;
  if (m_sliceAxis == neutube::EAxis::ARB) {
    result = DataToStack(pt, m_ap);
  } else {
    result = DataToStack(pt, m_sliceAxis);
  }

  return result;
}

