#include "zslicepainter.h"

#include <cmath>

#include "neulib/core/utilities.h"
#include "common/utilities.h"
//#include "geometry/zcuboid.h"
#include "geometry/zgeometry.h"
#include "geometry/zlinesegment.h"
#include "qt/gui/utilities.h"
#include "vis2d/utilities.h"

ZSlice2dPainter::ZSlice2dPainter()
{

}

void ZSlice2dPainter::preparePainter(QPainter *painter) const
{
  painter->setTransform(neutu::vis2d::GetPainterTransform(m_viewPlaneTransform));
}

void ZSlice2dPainter::drawCircle(
    QPainter *painter, double cx, double cy, double r) const
{
  if (painter && r > 0.0) {
    neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});
    preparePainter(painter);
    neutu::DrawCircle(*painter, cx, cy, r, neutu::PixelCentered(true));
  }
}

void ZSlice2dPainter::drawLine(
    QPainter *painter, double x0, double y0, double x1, double y1) const
{
  if (painter) {
    neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});
    preparePainter(painter);
    neutu::DrawLine(*painter, x0, y0, x1, y1, neutu::PixelCentered(true));
  }
}

void ZSlice2dPainter::drawPoint(QPainter *painter, double x, double y) const
{
  if (painter) {
    neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});
    preparePainter(painter);
    neutu::DrawPoint(*painter, x, y, neutu::PixelCentered(true));
  }
}

void ZSlice2dPainter::drawRect(
    QPainter *painter, double cx, double cy, double r) const
{
  drawRect(painter, cx -r, cy -r, cx + r, cy + r);
}

void ZSlice2dPainter::drawRect(
    QPainter *painter, double x0, double y0, double x1, double y1) const
{
  if (painter) {
    neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});
    preparePainter(painter);
    neutu::DrawRect(*painter, x0, y0, x1, y1, neutu::PixelCentered(true));
  }
}

void ZSlice2dPainter::drawImage(QPainter *painter, const QImage &image)
{
  if (painter) {
    neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});
    preparePainter(painter);
    painter->drawImage(0, 0, image);
  }
}

//ZSlice3dPainter

ZSlice3dPainter::ZSlice3dPainter()
{

}

void ZSlice3dPainter::setViewPlaneTransform(const ZViewPlaneTransform &t)
{
  m_painterHelper.setViewPlaneTransform(t);
}

namespace {

double adjusted_depth(double dz, double r, double depthScale)
{
  if (depthScale <= 0.0) {
     if (dz < 0.5) { //in focus
       dz = 0.0;
     } else {
       dz = r + 1.0; //make sure dz is greater than r to avoid painting
     }
  } else {
    dz /= depthScale;
  }

  return dz;
}

}

void ZSlice3dPainter::drawBoundBox(
    QPainter *painter, double cx, double cy, double cz, double r,
    double depthScale)
{
  ZPoint newCenter = m_worldViewTransform.transform(ZPoint(cx, cy, cz));
  double dz = std::fabs(newCenter.getZ() - m_worldViewTransform.getCutDepth());
  dz = adjusted_depth(dz, r, depthScale);

  if (dz < r) {
    m_painterHelper.drawRect(painter, newCenter.getX(), newCenter.getY(), r);
  } else {
    neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});
    neutu::RevisePen(painter, [](QPen &pen) {
      pen.setStyle(Qt::DashLine);
    });
    m_painterHelper.drawRect(painter, newCenter.getX(), newCenter.getY(), r);
  }
}

void ZSlice3dPainter::drawBall(
    QPainter *painter, double cx, double cy, double cz, double r,
    double depthScale, double fadingFactor) const
{
  if (painter) {
    ZPoint newCenter = m_worldViewTransform.transform(ZPoint(cx, cy, cz));
    double dz = std::fabs(newCenter.getZ() - m_worldViewTransform.getCutDepth());
    dz = adjusted_depth(dz, r, depthScale);

    if (dz < r) {
      double adjustedRadius = r;
      adjustedRadius = std::sqrt(r * r  - dz * dz);
      neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});
      if (fadingFactor > 0.0 && dz > 0.0) {
        double fading = (1.0 - fadingFactor * dz / r);
        QPen pen = painter->pen();
        QColor penColor = pen.color();
        penColor.setAlphaF(penColor.alphaF() * fading);
        pen.setColor(penColor);
        painter->setPen(pen);

        QBrush brush = painter->brush();
        QColor brushColor = brush.color();
        brushColor.setAlphaF(brushColor.alphaF() * fading);
        brush.setColor(brushColor);
        painter->setBrush(brush);
      }
      m_painterHelper.drawCircle(
            painter, newCenter.getX(), newCenter.getY(), adjustedRadius);
      if (dz == 0.0) {
        m_painterHelper.drawPoint(painter, newCenter.getX(), newCenter.getY());
      }
    }
  }
}

void ZSlice3dPainter::drawLine(QPainter *painter, const ZLineSegment &line)
{
  ZAffinePlane plane = m_worldViewTransform.getCutPlane();

  ZPoint v0 = plane.align(line.getStartPoint());
  ZPoint v1 = plane.align(line.getEndPoint());

  if (std::fabs(v0.getZ()) <= 0.5 && std::fabs(v1.getZ()) <= 0.5) {
    m_painterHelper.drawLine(
          painter, v0.getX(), v0.getY(), v1.getX(), v1.getY());
  } else {
    ZAffinePlane upperPlane = plane;
    upperPlane.translateDepth(-0.5);

    ZAffinePlane lowerPlane = plane;
    lowerPlane.translateDepth(0.5);

    double lambda1 = zgeom::ComputeIntersection(upperPlane, line);
    double lambda2 = zgeom::ComputeIntersection(lowerPlane, line);

    if (!((lambda1 < 0.0 && lambda2 < 0.0) || (lambda1 > 1.0 && lambda2 > 1.0))) {
      lambda1 = neulib::ClipValue(lambda1, 0.0, 1.0);
      lambda2 = neulib::ClipValue(lambda2, 0.0, 1.0);

      ZLineSegment alignedSeg(v0, v1);
      if (v0.getZ() > v1.getZ()) {
        std::swap(v0, v1);
        std::swap(lambda1, lambda2);
      }
      ZPoint fv0 = v0;
      ZPoint fv1 = v1;

      double length = std::sqrt(v0.getX() * v0.getX() + v1.getY() * v1.getY());
      double upperLength = length * lambda1;
      double lowerLength = length * (1.0 - lambda2);

      neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});


      if (upperLength >= 1.0) {
        fv0 = alignedSeg.getIntercept(lambda1);
      }

      if (lowerLength >= 1.0) {
        fv1 = alignedSeg.getIntercept(lambda2);
      }

      m_painterHelper.drawLine(
            painter, fv0.getX(), fv0.getY(), fv1.getX(), fv1.getY());

      if (lowerLength > 1.0 || upperLength >1.0) {
        neutu::ScalePenAlpha(painter, 0.5);

        if (upperLength > 1.0) {
          m_painterHelper.drawLine(
                painter, v0.getX(), v0.getY(), fv0.getX(), fv0.getY());
        }
        if (lowerLength > 1.0) {
          neutu::RevisePen(painter, [&](QPen &pen) {
            pen.setStyle(Qt::DashLine);
          });
          m_painterHelper.drawLine(
                painter, fv1.getX(), fv1.getY(), v1.getX(), v1.getY());
        }
      }
    }
  }
}

void ZSlice3dPainter::setCutPlane(const ZAffinePlane &plane)
{
  m_worldViewTransform.setCutPlane(plane);
}

void ZSlice3dPainter::setCutPlane(neutu::EAxis sliceAxis, double cutDepth)
{
  m_worldViewTransform.setCutPlane(sliceAxis, cutDepth);
}




