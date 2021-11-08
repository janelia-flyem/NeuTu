#include "zslicepainter.h"

#include <cmath>
#include <QRectF>
#include <QLineF>

#include "neulib/core/utilities.h"
#include "common/utilities.h"
#include "common/math.h"

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
          QRectF(QPointF(x0, y0), QPointF(x1 + 1, y1 + 1)));
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

void ZSlice2dPainter::drawCircleBase(
    QPainter *painter, double cx, double cy, double r,
    std::function<void(QPainter*)> drawFunc) const
{
  auto pred = [&](QPainter *painter) {
    return (r > 0.0 && intersects(painter, cx, cy, r));
  };

  draw(painter, drawFunc, pred);
}

void ZSlice2dPainter::drawCircle(
    QPainter *painter, double cx, double cy, double r) const
{
  auto drawFunc = [&](QPainter *painter) {
    neutu::DrawCircle(*painter, cx, cy, r, m_pixelCentered);
  };

  drawCircleBase(painter, cx, cy, r, drawFunc);
}

void ZSlice2dPainter::drawArc(
    QPainter *painter, double cx, double cy, double r,
    double startAngle, double spanAngle) const
{
  auto drawFunc = [&](QPainter *painter) {
    neutu::DrawArc(*painter, cx, cy, r, startAngle, spanAngle, m_pixelCentered);
  };

  drawCircleBase(painter, cx, cy, r, drawFunc);
}

void ZSlice2dPainter::drawStar(
    QPainter *painter, double cx, double cy, double r) const
{
  auto drawFunc = [&](QPainter *painter) {
    neutu::DrawStar(*painter, cx, cy, r, m_pixelCentered);
  };

  drawCircleBase(painter, cx, cy, r, drawFunc);
}

void ZSlice2dPainter::drawTriangle(
    QPainter *painter, double cx, double cy, double r,
    neutu::ECardinalDirection direction) const
{
  auto drawFunc = [&](QPainter *painter) {
    neutu::DrawTriangle(
          *painter, cx, cy, r, direction, m_pixelCentered);
  };

  drawCircleBase(painter, cx, cy, r, drawFunc);
}

void ZSlice2dPainter::drawCross(
    QPainter *painter, double cx, double cy, double r) const
{
  auto drawFunc = [&](QPainter *painter) {
    neutu::DrawCross(*painter, cx, cy, r, m_pixelCentered);
  };

  drawCircleBase(painter, cx, cy, r, drawFunc);
  /*
  auto pred = [&](QPainter *painter) {
    return (r > 0.0 && intersects(painter, cx, cy, r));
  };

  draw(painter, drawFunc, pred);
  */
}

void ZSlice2dPainter::drawLine(
    QPainter *painter, double x0, double y0, double x1, double y1) const
{
  auto drawFunc = [&](QPainter *painter) {
    neutu::DrawLine(*painter, x0, y0, x1, y1, m_pixelCentered);
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

void ZSlice2dPainter::drawPolyline(
    QPainter *painter, const std::vector<QPointF> &points) const
{
  auto drawFunc = [&](QPainter *painter) {
    neutu::DrawPolyline(*painter, points, m_pixelCentered);
  };

  auto pred = [&](QPainter */*painter*/) {
    return true;
  };

  draw(painter, drawFunc, pred);
}

void ZSlice2dPainter::drawPolyline(
    QPainter *painter, const std::vector<QPoint> &points) const
{
  auto drawFunc = [&](QPainter *painter) {
    neutu::DrawPolyline(*painter, points, m_pixelCentered);
  };

  auto pred = [&](QPainter */*painter*/) {
    return true;
  };

  draw(painter, drawFunc, pred);
}

void ZSlice2dPainter::drawPoint(QPainter *painter, double x, double y) const
{
  auto drawFunc = [&](QPainter *painter) {
    neutu::DrawPoint(*painter, x, y, m_pixelCentered);
  };

  auto pred = [&](QPainter *painter) {
    return intersects(painter, x, y);
  };

  draw(painter, drawFunc, pred);
}

void ZSlice2dPainter::drawPoints(
    QPainter *painter, const std::vector<QPointF> &points) const
{
  auto drawFunc = [&](QPainter *painter) {
    neutu::DrawPoints(*painter, points, m_pixelCentered);
  };

  auto pred = [&](QPainter */*painter*/) {
    return true;
  };

  draw(painter, drawFunc, pred);
}

void ZSlice2dPainter::drawPoints(
    QPainter *painter, const std::vector<QPoint> &points) const
{
  auto drawFunc = [&](QPainter *painter) {
    neutu::DrawPoints(*painter, points, m_pixelCentered);
  };

  auto pred = [&](QPainter */*painter*/) {
    return true;
  };

  draw(painter, drawFunc, pred);
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
    neutu::DrawRect(*painter, x0, y0, x1, y1, m_pixelCentered);
  };

  auto pred = [&](QPainter *painter) {
    return intersects(painter, x0, y0, x1, y1);
  };

  draw(painter, drawFunc, pred);
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

void ZSlice2dPainter::drawLines(
    QPainter *painter, const std::vector<QLineF> &lines) const
{
  if (!lines.empty()) {
    auto drawFunc = [&](QPainter *painter) {
      neutu::DrawLines(*painter, lines, m_pixelCentered);
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
}
/*
void ZSlice2dPainter::drawLines(
    QPainter *painter, const std::vector<QLineF> &lines) const
{
  QVector<QLineF> lineArray;
  for (const QLineF &line : lines) {
    lineArray.append(line);
  }
  drawLines(painter, lineArray);
}
*/

void ZSlice2dPainter::drawLines(
    QPainter *painter, const std::vector<double> &lines) const
{
  if (lines.size() % 4 == 0) {
    std::vector<QLineF> lineArray(lines.size() / 4);
    for (size_t i = 0; i < lineArray.size(); ++i) {
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

void ZSlice2dPainter::setPixelCentered(bool on)
{
  m_pixelCentered = on;
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
    neutu::ECardinalDirection direction = neutu::ECardinalDirection::NORTH;
    if (newCenter.getZ() > 0) {
      direction = neutu::ECardinalDirection::SOUTH;
    }

    m_painterHelper.drawTriangle(
          painter, newCenter.getX(), newCenter.getY(), r, direction);
//    m_painterHelper.drawRect(painter, newCenter.getX(), newCenter.getY(), r);
  }
}

void ZSlice3dPainter::drawBall(
    QPainter *painter, const ZPoint &center, double r,
    double depthScale, double fadingFactor,
    std::function<void(
      QPainter *painter, const ZSlice2dPainter &painterHelper,
      double cx, double cy, double dz, double r)> paintDecor) const
{
  drawBall(painter, center.x(), center.y(), center.z(), r,
           depthScale, fadingFactor, paintDecor);
}

bool ZSlice3dPainter::BallHitTest(
    double x, double y, double z, double cx, double cy, double cz, double r,
    const ZModelViewTransform &transform, double depthScale)
{
  ZPoint newPoint = transform.transform({x, y, z});
  ZPoint newCenter = transform.transform({cx, cy, cz});
  ZPoint dp = newPoint - newCenter;
  double dz = adjusted_depth(std::fabs(dp.getZ()), r, depthScale);
  if (dz < r) {
    double adjustedRadiusSquare = r * r  - dz * dz;
    return dp.getX() * dp.getX() + dp.getY() * dp.getY() < adjustedRadiusSquare;
  }

  return false;
}

std::function<bool(double,double,double)>
ZSlice3dPainter::getBallHitFunc(
    double cx, double cy, double cz, double r, double depthScale) const
{
  auto transform = m_worldViewTransform;
  return [=](double x, double y, double z) {
    return BallHitTest(x, y, z, cx, cy, cz, r, transform, depthScale);
  };
}

void ZSlice3dPainter::prepareBallDrawing(
    QPainter *painter, double dz, double r, double fadingFactor) const
{
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
}

void ZSlice3dPainter::drawBall(
    QPainter *painter, double cx, double cy, double cz, double r,
    double depthScale, double fadingFactor,
    std::function<void(
      QPainter *painter, const ZSlice2dPainter &painterHelper,
      double cx, double cy, double dz, double r)> paintDecor) const
{
  if (painter) {
    ZPoint newCenter = m_worldViewTransform.transform(ZPoint(cx, cy, cz));
    double dz = std::fabs(newCenter.getZ());
    dz = adjusted_depth(dz, r, depthScale);

    if (dz < r) {
      double adjustedRadius = std::sqrt(r * r  - dz * dz);
      neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});
      prepareBallDrawing(painter, dz, r, fadingFactor);
#ifdef _DEBUG_2
      std::cout << "drawBall newCenter: " << newCenter.toString() << std::endl;
#endif
      m_painterHelper.drawCircle(
            painter, newCenter.getX(), newCenter.getY(), adjustedRadius);
      if (dz == 0.0) {
        m_painterHelper.drawPoint(painter, newCenter.getX(), newCenter.getY());
      }
      if (paintDecor) {
        paintDecor(painter, m_painterHelper, newCenter.getX(), newCenter.getY(),
                   dz, adjustedRadius);
      }
    }
  }
}

void ZSlice3dPainter::drawCross(
    QPainter *painter, double cx, double cy, double cz, double r,
    double depthScale, double fadingFactor) const
{
  if (painter) {
    ZPoint newCenter = m_worldViewTransform.transform(ZPoint(cx, cy, cz));
    double dz = std::fabs(newCenter.getZ());
    dz = adjusted_depth(dz, r, depthScale);

    if (dz < r) {
      double adjustedRadius = std::sqrt(r * r  - dz * dz);
      neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});
      prepareBallDrawing(painter, dz, r, fadingFactor);
      m_painterHelper.drawCross(
            painter, newCenter.getX(), newCenter.getY(), adjustedRadius);
    }
  }
}

void ZSlice3dPainter::drawCross(
    QPainter *painter, const ZPoint &center, double r,
    double depthScale, double fadingFactor) const
{
  drawCross(painter, center.getX(), center.getY(), center.getZ(), r,
            depthScale, fadingFactor);
}

void ZSlice3dPainter::drawStar(
    QPainter *painter, const ZPoint &center, double r,
    double depthScale, double fadingFactor) const
{
  drawStar(painter, center.getX(), center.getY(), center.getZ(), r,
           depthScale, fadingFactor);
}

void ZSlice3dPainter::drawStar(
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
#ifdef _DEBUG_2
      std::cout << "drawBall newCenter: " << newCenter.toString() << std::endl;
#endif
      m_painterHelper.drawStar(
            painter, newCenter.getX(), newCenter.getY(), adjustedRadius);
      if (dz == 0.0) {
        m_painterHelper.drawPoint(painter, newCenter.getX(), newCenter.getY());
      }
    }
  }
}

void ZSlice3dPainter::drawLineProjection(
    QPainter *painter, const ZLineSegment &line)
{
  ZAffinePlane plane = m_worldViewTransform.getCutPlane();

  ZPoint v0 = plane.align(line.getStartPoint());
  ZPoint v1 = plane.align(line.getEndPoint());

  m_painterHelper.drawLine(
        painter, v0.getX(), v0.getY(), v1.getX(), v1.getY());
}

void ZSlice3dPainter::drawBallProjection(
    QPainter *painter, double cx, double cy, double cz, double r)
{
  ZPoint newCenter = m_worldViewTransform.getCutPlane().align({cx, cy, cz});
  m_painterHelper.drawCircle(painter, newCenter.getX(), newCenter.getY(), r);
}

void ZSlice3dPainter::drawCrossProjection(
    QPainter *painter, double cx, double cy, double cz, double r)
{
  ZPoint newCenter = m_worldViewTransform.getCutPlane().align({cx, cy, cz});
  m_painterHelper.drawCross(painter, newCenter.getX(), newCenter.getY(), r);
}

void ZSlice3dPainter::drawRectProjection(
    QPainter *painter, double x0, double y0, double x1, double y1, double z)
{
  ZPoint minCorner = m_worldViewTransform.getCutPlane().align({x0, y0, z});
  ZPoint maxCorner = m_worldViewTransform.getCutPlane().align({x1, y1, z});

  m_painterHelper.drawRect(
        painter, minCorner.getX(), minCorner.getY(),
        maxCorner.getX(), maxCorner.getY());
}

void ZSlice3dPainter::drawLine(QPainter *painter, const ZLineSegment &line)
{
  ZAffinePlane plane = m_worldViewTransform.getCutPlane();

  ZPoint vs = plane.align(line.getStartPoint());
  ZPoint vd = plane.align(line.getEndPoint());

  if (std::fabs(vs.getZ()) <= 0.5 && std::fabs(vd.getZ()) <= 0.5) {
    m_painterHelper.drawLine(
          painter, vs.getX(), vs.getY(), vd.getX(), vd.getY());
  } else {
    ZAffinePlane shallowPlane = plane;
    shallowPlane.translateDepth(-0.5);

    ZAffinePlane deepPlane = plane;
    deepPlane.translateDepth(0.5);

    double slambda = zgeom::ComputeIntersection(shallowPlane, line);
    double dlambda = zgeom::ComputeIntersection(deepPlane, line);

    if (!((slambda < 0.0 && dlambda < 0.0) || (slambda > 1.0 && dlambda > 1.0))) {
      slambda = neulib::ClipValue(slambda, 0.0, 1.0);
      dlambda = neulib::ClipValue(dlambda, 0.0, 1.0);

      ZLineSegment alignedSeg(vs, vd);
      ZPoint fvs = alignedSeg.getIntercept(slambda);
      ZPoint fvd = alignedSeg.getIntercept(dlambda);

      if (vs.getZ() > vd.getZ()) {
        std::swap(vs, vd);
        slambda = 1.0 - slambda;
        dlambda = 1.0 - dlambda;
//        std::swap(lambda1, lambda2);
      }

      double length = std::sqrt(vs.getX() * vs.getX() + vd.getY() * vd.getY());
      double shallowLength = length * slambda;
      double deepLength = length * (1.0 - dlambda);

      neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});

      /*
      if (upperLength >= 1.0) {
        fv0 = alignedSeg.getIntercept(lambda1);
      }

      if (lowerLength >= 1.0) {
        fv1 = alignedSeg.getIntercept(lambda2);
      }
      */

      m_painterHelper.drawLine(
            painter, fvs.getX(), fvs.getY(), fvd.getX(), fvd.getY());

      if (deepLength > 1.0 || shallowLength >1.0) {
        neutu::ScalePenAlpha(painter, 0.3);
        if (shallowLength > 1.0) {
          m_painterHelper.drawLine(
                painter, vs.getX(), vs.getY(), fvs.getX(), fvs.getY());
        }

        if (deepLength > 1.0) {
          neutu::ScalePenAlpha(painter, 2.0);
          neutu::RevisePen(painter, [&](QPen &pen) {
            pen.setStyle(Qt::DashLine);
          });
          m_painterHelper.drawLine(
                painter, fvd.getX(), fvd.getY(), vd.getX(), vd.getY());
        }
      }
    }
  }
}

void ZSlice3dPainter::drawLine(
    QPainter *painter, const ZLineSegment &line, double r0, double r1)
{
  ZAffinePlane plane = m_worldViewTransform.getCutPlane();

  ZPoint v0 = plane.align(line.getStartPoint());
  ZPoint v1 = plane.align(line.getEndPoint());

  if ((v0.getZ() * v1.getZ() <= 0.0) ||
      ((std::fabs(v0.getZ()) < 0.5 || std::fabs(v1.getZ()) < 0.5))) {
    drawLine(painter, line);
  } else {
    if (std::fabs(v0.getZ()) < r0 || std::fabs(v1.getZ()) < r1) {
      neutu::ScalePenAlpha(painter, 0.5);
      if (v0.getZ() > 0.0) {
        neutu::RevisePen(painter, [&](QPen &pen) {
          pen.setStyle(Qt::DashLine);
        });
      }
      m_painterHelper.drawLine(
            painter, v0.getX(), v0.getY(), v1.getX(), v1.getY());
    }
  }
}

void ZSlice3dPainter::drawPoint(QPainter *painter, double x, double y, double z)
{
  if (painter) {
    ZPoint newCenter = m_worldViewTransform.transform(ZPoint(x, y, z));
    double dz = newCenter.getZ();

    if (dz >= -0.5 && dz < 0.5) {
//      neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});
      m_painterHelper.drawPoint(painter, newCenter.getX(), newCenter.getY());
    }
  }
}

void ZSlice3dPainter::drawPoints(
    QPainter *painter, const std::vector<ZPoint> &points) const
{
  std::vector<QPointF> newPoints;
  for (const ZPoint &pt : points) {
    ZPoint newPt = m_worldViewTransform.transform(pt);
    if (newPt.getZ() >= -0.5 && newPt.getZ() < 0.5) {
      newPoints.push_back(QPointF(newPt.getX(), newPt.getY()));
    }
  }

  if (!newPoints.empty()) {
    neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});
    m_painterHelper.drawPoints(painter, newPoints);
  }
}

void ZSlice3dPainter::drawPlanePolyline(
    QPainter *painter, const std::vector<QPointF> &points,
    double z, neutu::EAxis sliceAxis) const
{
  if (sliceAxis != neutu::EAxis::ARB &&
      m_worldViewTransform.getSliceAxis() == sliceAxis &&
      !points.empty()) {
    ZPoint shiftedCenter = m_worldViewTransform.getCutCenter();
    shiftedCenter.shiftSliceAxis(sliceAxis);
    if (neutu::nround(z - shiftedCenter.getZ()) == 0) {
      std::vector<QPointF> newPoints = points;
      double x0 = shiftedCenter.getX();
      double y0 = shiftedCenter.getY();
      for (QPointF &pt : newPoints) {
        pt.rx() -= x0;
        pt.ry() -= y0;
      }
      m_painterHelper.drawPolyline(painter, newPoints);
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




