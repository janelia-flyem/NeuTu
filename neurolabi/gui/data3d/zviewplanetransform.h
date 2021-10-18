#ifndef ZVIEWPLANETRANSFORM_H
#define ZVIEWPLANETRANSFORM_H

#include <tuple>
#include <iostream>

#include "geometry/2d/rectangle.h"
#include "geometry/2d/point.h"

class ZPoint;
class ZJsonObject;

/*!
 * \brief Aspect-ratio-preserved scaling-translation transform
 *
 * xs = xv * s + dx
 */
class ZViewPlaneTransform
{
public:
  ZViewPlaneTransform();

  void set(double dx, double dy, double s);

  double getScale() const;
  double getTx() const;
  double getTy() const;

  void setScale(double s);
  void incScale();
  void decScale();
  double getScaleStep() const;
  void setOffset(double tx, double ty);

  neutu::geom2d::Point transform(double x, double y) const;
  neutu::geom2d::Point transform(const neutu::geom2d::Point &pt) const;
  neutu::geom2d::Rectangle transform(const neutu::geom2d::Rectangle &rect) const;
  void transform(double *x, double *y) const;

  /*!
   * \brief Transform a 3D point
   *
   * It transforms X and Y coordinates \a pt, while keeping Z unchanged. It
   * servs as a convenient API for working with a 3D point.
   */
  ZPoint transform(const ZPoint &pt) const;

  void inverseTransform(double *x, double *y) const;

  ZPoint inverseTransform(const ZPoint &pt) const;

  neutu::geom2d::Point inverseTransform(const neutu::geom2d::Point &pt) const;
  neutu::geom2d::Rectangle inverseTransform(
      const neutu::geom2d::Rectangle &rect) const;

  /*!
   * \brief Adjust the transform to fit a rectangle in the target area
   *
   * The resulted transform maps the rectangle of \a srcWidth x \a srcHeight
   * at center (\a cu, \a cv) to (0, 0, \a dstWidth, \a dstHeight) by maximizing
   * the mapped size and aligning the centers while containing the mapped
   * rectangle completely inside the target region.
   *
   * Nothing will be done if \a srcWidth or \a srcHeight is not positive.
   */
  void centerFit(double cu, double cv, double srcWidth, double srcHeight,
      double dstWidth, double dstHeight);

  void centerFit(
      const neutu::geom2d::Point &center, double srcWidth, double srcHeight,
      double dstWidth, double dstHeight);

  /*!
   * \brief Adjust the transform to fit a rectangle in the target area
   *
   * The center of the source rectangle is matched to the center of the center
   * of the target
   */
  void centerFit(const neutu::geom2d::Rectangle &src,
                 const neutu::geom2d::Rectangle &dst);

  void centerFit(double srcWidth, double srcHeight,
           double dstWidth, double dstHeight);

  /*!
   * \brief Change the scale with the map of a given point fixed
   *
   * (\a u, \a v) is mapped to the same point as before after changing the
   * scale to s.
   */
  void setScaleFixingOriginal(double s, double u, double v);

  void setScaleFixingMapped(double s, double a, double b);

  void setMinScale(double s);
  double getMinScale() const;
  void setMaxScale(double s);
  double getMaxScale() const;

  bool canZoomIn() const;
  bool canZoomOut() const;

  /*!
   * \brief Change translation to match point mapping
   *
   * Change the translation part of the tranform only to map (\a u, \a v)
   * to (\a a, \a b).
   */
  void translateTransform(double u, double v, double a, double b);

  void translateTransform(const neutu::geom2d::Point &src,
                          const neutu::geom2d::Point &dst);

  ZJsonObject toJsonObject() const;

  bool operator==(const ZViewPlaneTransform &t) const;
//  QTransform getPainterTransform() const;

  friend std::ostream& operator<< (
      std::ostream &stream, const ZViewPlaneTransform &t);

private:
  void clipScale();

private:
  double m_dx = 0.0;
  double m_dy = 0.0;
  double m_s = 1.0;

  double m_minScale = 0.000001;
  double m_maxScale = 100.0;
};

#endif // ZVIEWPLANETRANSFORM_H
