#ifndef ZSLICEPAINTER_H
#define ZSLICEPAINTER_H

#include "data3d/zworldviewtransform.h"
#include "data3d/zviewplanetransform.h"

class QPainter;
class QImage;
class ZLineSegment;
//class ZCuboid;

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

  void drawCircle(QPainter *painter, double cx, double cy, double r) const;
  void drawLine(
      QPainter *painter, double x0, double y0, double x1, double y1) const;
  void drawPoint(QPainter *painter, double x, double y) const;
  void drawRect(
      QPainter *painter, double x0, double y0, double x1, double y1) const;
  void drawRect(
      QPainter *painter, double cx, double cy, double r) const;
  void drawImage(QPainter *painter, const QImage &image);

private:
  void preparePainter(QPainter *painter) const;

private:
  ZViewPlaneTransform m_viewPlaneTransform;
};

class ZSlice3dPainter
{
public:
  ZSlice3dPainter();

  void setViewPlaneTransform(const ZViewPlaneTransform &t);

  /*!
   * \brief Draw a ball on a slice
   *
   * When depthScale is 0, only the slice that is in focus is painted.
   */
  void drawBall(
      QPainter *painter, double cx, double cy, double cz, double r,
      double depthScale, double fadingFactor) const;
  void drawBoundBox(
      QPainter *painter, double cx, double cy, double cz, double r,
      double depthScale);
  void drawLine(QPainter *painter, const ZLineSegment &line);

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

private:
  ZSlice2dPainter m_painterHelper;
  ZWorldViewTransform m_worldViewTransform;
};

#endif // ZSLICEPAINTER_H
