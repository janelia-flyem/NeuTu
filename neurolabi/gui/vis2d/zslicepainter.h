#ifndef ZSLICEPAINTER_H
#define ZSLICEPAINTER_H

#include <vector>
#include <utility>
#include <functional>

#include <QVector>

#include "data3d/zmodelviewtransform.h"
#include "data3d/zviewplanetransform.h"
//#include "data3d/displayconfig.h"

class QPainter;
class QImage;
class QLineF;
class QPoint;
class QPointF;
class ZLineSegment;
class ZPoint;

/*!
 * \brief The class painting a 2d primitive on a slice
 */
class ZSlice2dPainter
{
public:
  ZSlice2dPainter();

  void setViewPlaneTransform(const ZViewPlaneTransform &t) {
    m_viewPlaneTransform = t;
  }

  void draw(
      QPainter *painter, std::function<void(QPainter *painter)> drawFunc,
      std::function<bool(QPainter *painter)> pred) const;

  void drawCircle(QPainter *painter, double cx, double cy, double r) const;
  void drawStar(QPainter *painter, double cx, double cy, double r) const;
  void drawCross(QPainter *painter, double cx, double cy, double r) const;
  void drawLine(
      QPainter *painter, double x0, double y0, double x1, double y1) const;
  void drawLines(
      QPainter *painter, const std::vector<double> &lines) const;
//  void drawLines(QPainter *painter, const QVector<QLineF> &lines) const;
  void drawLines(QPainter *painter, const std::vector<QLineF> &lines) const;
  void drawPolyline(QPainter *painter, const std::vector<QPointF> &points) const;
  void drawPolyline(QPainter *painter, const std::vector<QPoint> &points) const;
  void drawPoint(QPainter *painter, double x, double y) const;
  void drawPoints(QPainter *painter, const std::vector<QPointF> &points) const;
  void drawPoints(QPainter *painter, const std::vector<QPoint> &points) const;
  void drawRect(
      QPainter *painter, double x0, double y0, double x1, double y1) const;
  void drawRect(
      QPainter *painter, double cx, double cy, double r) const;
  void drawImage(QPainter *painter, const QImage &image);

  bool getPaintedHint() const;
  void setPaintedHint(bool painted) const;

private:
  void preparePainter(QPainter *painter) const;
  bool intersects(QPainter *painter, double x, double y) const;
  bool intersects(QPainter *painter, double x0, double y0, double x1, double y1) const;
  bool intersects(QPainter *painter, double x, double y, double r) const;
  void drawCircleBase(
      QPainter *painter, double cx, double cy, double r,
      std::function<void(QPainter*)> drawFunc) const;

private:
  ZViewPlaneTransform m_viewPlaneTransform;
  mutable bool m_painted = false;
};

class ZSlice3dPainter
{
public:
  ZSlice3dPainter();

  void setViewCanvasTransform(const ZViewPlaneTransform &t);
  void setModelViewTransform(const ZModelViewTransform &t);
  void setTransform(const ZModelViewTransform &tmv, const ZViewPlaneTransform &tvc);

  /*!
   * \brief Draw a ball on a slice
   *
   * When depthScale is 0, only the slice that is in focus is painted.
   */
  void drawBall(
      QPainter *painter, double cx, double cy, double cz, double r,
      double depthScale, double fadingFactor) const;
  void drawBall(
      QPainter *painter, const ZPoint &center, double r,
      double depthScale, double fadingFactor) const;

  void drawCross(
      QPainter *painter, double cx, double cy, double cz, double r,
      double depthScale, double fadingFactor) const;
  void drawCross(
      QPainter *painter, const ZPoint &center, double r,
      double depthScale, double fadingFactor) const;

  void drawStar(
      QPainter *painter, double cx, double cy, double cz, double r,
      double depthScale, double fadingFactor) const;
  void drawStar(
      QPainter *painter, const ZPoint &center, double r,
      double depthScale, double fadingFactor) const;
  void drawBoundBox(
      QPainter *painter, double cx, double cy, double cz, double r,
      double depthScale);
  void drawBoundBox(
      QPainter *painter, const ZPoint &center, double r, double depthScale);
  void drawLine(QPainter *painter, const ZLineSegment &line);
  void drawLines(QPainter *painter, const std::vector<ZLineSegment> &lines);
  void drawLine(
      QPainter *painter, const ZLineSegment &line, double r0, double r1);
  void drawPoint(QPainter *painter, double x, double y, double z);
  void drawPoints(QPainter *painter, const std::vector<ZPoint> &points) const;

  void drawPlanePolyline(
      QPainter *painter, const std::vector<QPointF> &points,
      double z, neutu::EAxis sliceAxis) const;

  void drawLineProjection(QPainter *painter, const ZLineSegment &line);
  void drawBallProjection(
      QPainter *painter, double cx, double cy, double cz, double r);
  void drawCrossProjection(
      QPainter *painter, double cx, double cy, double cz, double r);
  void drawRectProjection(
      QPainter *painter, double x0, double y0, double x1, double y1, double z);

  /*!
   * \brief Set cut plane
   *
   * The slice axis will be set to neutu::EAxis::ARB automatically.
   */
  void setCutPlane(const ZAffinePlane &plane);

  /*!
   * \brief Set cut plane
   *
   * It will move the cut plane if the slice axis is eutu::EAxis::ARB.
   */
  void setCutPlane(neutu::EAxis sliceAxis, double cutDepth);

  bool getPaintedHint() const;

  std::function<bool(double,double,double)>
  getBallHitFunc(
      double cx, double cy, double cz, double r, double depthScale) const;

  static bool BallHitTest(
      double x, double y, double z,
      double cx, double cy, double cz, double r,
      const ZModelViewTransform &transform, double depthScale);

private:
  void prepareBallDrawing(
      QPainter *painter, double dz, double r, double fadingFactor) const;

private:
  ZSlice2dPainter m_painterHelper;
  ZModelViewTransform m_worldViewTransform;
};

#endif // ZSLICEPAINTER_H
