#include "zslicepainter.h"

#include <cmath>
#include <QRectF>
#include <QLineF>

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

namespace {

QRectF get_canvas_rectf(const QPainter &painter) {
  return QRectF(
        QPointF(-0.5, -0.5),
        QPointF(painter.device()->width() + 0.5,
                painter.device()->height() + 0.5));
}

}

bool ZSlice2dPainter::intersects(QPainter *painter, double x, double y) const
{
  if (painter) {
    m_viewPlaneTransform.transform(&x, &y);

    return get_canvas_rectf(*painter).contains(x, y);
  }

  return false;
}

bool ZSlice2dPainter::intersects(
    QPainter *painter, double x0, double y0, double x1, double y1) const
{
  if (painter) {
    m_viewPlaneTransform.transform(&x0, &y0);
    m_viewPlaneTransform.transform(&x1, &y1);
    return get_canvas_rectf(*painter).intersects(
          QRectF(QPointF(x0, y0), QPointF(x1, y1)));
  }

  return false;
}

bool ZSlice2dPainter::intersects(
    QPainter *painter, double x, double y, double r) const
{
  return intersects(painter, x - r, y - r, x + r, y + r);
}

void ZSlice2dPainter::draw(
    QPainter *painter, std::function<void(QPainter *painter)> drawFunc,
    std::function<bool(QPainter *painter)> pred) const
{
  if (painter) {
    if (pred(painter)) {
      neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});
      preparePainter(painter);
      drawFunc(painter);
      setPaintedHint(true);
    }
  }
}

void ZSlice2dPainter::drawCircle(
    QPainter *painter, double cx, double cy, double r) const
{
  auto drawFunc = [&](QPainter *painter) {
    neutu::DrawCircle(*painter, cx, cy, r, neutu::PixelCentered(true));
  };
  auto pred = [&](QPainter *painter) {
    return (r > 0.0 && intersects(painter, cx, cy, r));
  };

  draw(painter, drawFunc, pred);

  /*
  if (painter && r > 0.0) {
    if (intersects(painter, cx, cy, r)) {
      neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});
      preparePainter(painter);
      neutu::DrawCircle(*painter, cx, cy, r, neutu::PixelCentered(true));
      setPaintedHint(true);
    }
  }
  */
}

void ZSlice2dPainter::drawLine(
    QPainter *painter, double x0, double y0, double x1, double y1) const
{
  auto drawFunc = [&](QPainter *painter) {
    neutu::DrawLine(*painter, x0, y0, x1, y1, neutu::PixelCentered(true));
  };

  auto pred = [&](QPainter *painter) {
    return intersects(painter, x0, y0, x1, y1);
  };

  draw(painter, drawFunc, pred);

  /*
  if (painter && intersects(painter, x0, y0, x1, y1)) {
    neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});
    preparePainter(painter);
    neutu::DrawLine(*painter, x0, y0, x1, y1, neutu::PixelCentered(true));
    setPaintedHint(true);
  }
  */
}

void ZSlice2dPainter::drawPoint(QPainter *painter, double x, double y) const
{
  auto drawFunc = [&](QPainter *painter) {
    neutu::DrawPoint(*painter, x, y, neutu::PixelCentered(true));
  };

  auto pred = [&](QPainter *painter) {
    return intersects(painter, x, y);
  };

  draw(painter, drawFunc, pred);
  /*
  if (painter && intersects(painter, x, y)) {
    neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});
    preparePainter(painter);
    neutu::DrawPoint(*painter, x, y, neutu::PixelCentered(true));
    setPaintedHint(true);
  }*/
}

void ZSlice2dPainter::drawRect(
    QPainter *painter, double cx, double cy, double r) const
{
  drawRect(painter, cx -r, cy -r, cx + r, cy + r);
}

void ZSlice2dPainter::drawRect(
    QPainter *painter, double x0, double y0, double x1, double y1) const
{
  auto drawFunc = [&](QPainter *painter) {
    neutu::DrawRect(*painter, x0, y0, x1, y1, neutu::PixelCentered(true));
  };

  auto pred = [&](QPainter *painter) {
    return intersects(painter, x0, y0, x1, y1);
  };

  draw(painter, drawFunc, pred);

  /*
  if (painter && intersects(painter, x0, y0, x1, y1)) {
    neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});
    preparePainter(painter);
    neutu::DrawRect(*painter, x0, y0, x1, y1, neutu::PixelCentered(true));
    setPaintedHint(true);
  }
  */
}

void ZSlice2dPainter::drawImage(QPainter *painter, const QImage &image)
{
  auto drawFunc = [&](QPainter *painter) {
    painter->drawImage(0, 0, image);
  };

  auto pred = [&](QPainter *painter) {
    return intersects(painter, -0.5, -0.5, image.width() + 0.5, image.height() + 0.5);
  };

  draw(painter, drawFunc, pred);
  /*
  if (painter &&
      intersects(painter, -0.5, -0.5, image.width() + 0.5, image.height() + 0.5)) {
    neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});
    preparePainter(painter);
    painter->drawImage(0, 0, image);
    setPaintedHint(true);
  }
  */
}

void ZSlice2dPainter::drawLines(QPainter *painter, QVector<QLineF> &lines) const
{
  auto drawFunc = [&](QPainter *painter) {
    painter->drawLines(lines);
  };
  auto pred = [&](QPainter *painter) {
    for (const QLineF &line : lines) {
      if (intersects(painter, line.x1(), line.y1(), line.x2(), line.y2())) {
        return true;
      }
    }
    return false;
  };

  draw(painter, drawFunc, pred);
}

void ZSlice2dPainter::drawLines(
    QPainter *painter, const std::vector<double> &lines) const
{
  if (lines.size() % 4 == 0) {
    QVector<QLineF> lineArray(lines.size() / 4);
    for (int i = 0; i < lineArray.size(); ++i) {
      lineArray[i].setLine(lines[i*4], lines[i*4+1], lines[i*4+2], lines[i*4+3]);
    }

    drawLines(painter, lineArray);
  }

  /*
  if (painter) {
    neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});
    preparePainter(painter);
    QVector<QPointF> lineArray(lines.size());
    for (size_t i = 0; i < lines.size(); ++i) {
      lineArray[i] = QPointF(lines[i].first, lines[i].second);
    }
    painter->drawLines(lineArray);
    setPaintedHint(true);
  }
  */
}

bool ZSlice2dPainter::getPaintedHint() const
{
  return m_painted;
}

void ZSlice2dPainter::setPaintedHint(bool painted) const
{
  m_painted = painted;
}

//ZSlice3dPainter

ZSlice3dPainter::ZSlice3dPainter()
{

}

void ZSlice3dPainter::setViewCanvasTransform(const ZViewPlaneTransform &t)
{
  m_painterHelper.setViewPlaneTransform(t);
}

void ZSlice3dPainter::setModelViewTransform(const ZModelViewTransform &t)
{
  m_worldViewTransform = t;
}

void ZSlice3dPainter::setTransform(
    const ZModelViewTransform &tmv, const ZViewPlaneTransform &tvc)
{
  setModelViewTransform(tmv);
  setViewCanvasTransform(tvc);
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
    QPainter *painter, const ZPoint &center, double r, double depthScale)
{
  drawBoundBox(
        painter, center.getX(), center.getY(), center.getZ(), r, depthScale);
}

void ZSlice3dPainter::drawBoundBox(
    QPainter *painter, double cx, double cy, double cz, double r,
    double depthScale)
{
  ZPoint newCenter = m_worldViewTransform.transform(ZPoint(cx, cy, cz));
  double dz = std::fabs(newCenter.getZ());
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
    QPainter *painter, const ZPoint &center, double r,
    double depthScale, double fadingFactor) const
{
  drawBall(painter, center.x(), center.y(), center.z(), r,
           depthScale, fadingFactor);
}

void ZSlice3dPainter::drawBall(
    QPainter *painter, double cx, double cy, double cz, double r,
    double depthScale, double fadingFactor) const
{
  if (painter) {
    ZPoint newCenter = m_worldViewTransform.transform(ZPoint(cx, cy, cz));
    double dz = std::fabs(newCenter.getZ());
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

void ZSlice3dPainter::drawPoint(QPainter *painter, double x, double y, double z)
{
  if (painter) {
    ZPoint newCenter = m_worldViewTransform.transform(ZPoint(x, y, z));
    double dz = std::fabs(newCenter.getZ());

    if (dz < 0.5) {
      neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});
      m_painterHelper.drawPoint(painter, newCenter.getX(), newCenter.getY());
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

bool ZSlice3dPainter::getPaintedHint() const
{
  return m_painterHelper.getPaintedHint();
}




