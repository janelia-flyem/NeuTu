#ifndef ZSLICEVIEWTRANSFORM_H
#define ZSLICEVIEWTRANSFORM_H

#include "defs.h"

#include "geometry/2d/rectangle.h"
#include "geometry/2d/point.h"
#include "geometry/zintcuboid.h"

#include "zmodelviewtransform.h"
#include "zviewplanetransform.h"


class ZAffineRect;
class ZCuboid;

/*!
 * \brief The class for trasnforming a 3D object onto a 2D slice
 *
 * Symbols:
 * Model space: (x, y, z)
 * View space: (u, v, n)
 * Canvas space: (a, b)
 */
class ZSliceViewTransform
{
public:
  ZSliceViewTransform();
  ZSliceViewTransform(
      const ZModelViewTransform &tmv, const ZViewPlaneTransform &tvc);

  ZModelViewTransform getModelViewTransform() const;
  ZViewPlaneTransform getViewCanvasTransform() const;

  neutu::EAxis getSliceAxis() const;
  ZPoint getCutPlaneNormal() const;
  ZPoint getCutCenter() const;
  double getScale() const;
  ZAffineRect getCutRect(int canvasWidth, int canvasHeight) const;
  ZAffineRect getCutRect(
      int width, int height, neutu::data3d::ESpace sizeSpace) const;
  /*!
   * \brief Get cut rectangle with all integer paramters
   */
  ZAffineRect getIntCutRect(
      int width, int height, neutu::data3d::ESpace sizeSpace);

  ZAffinePlane getCutPlane() const;

  void setScale(double s);
  void incScale();
  void decScale();
  double getScaleStep() const;

  void setMinScale(double s);
  double getMinScale() const;
  double getMaxScale() const;

  void setCutCenter(const ZPoint &pt);
  void setCutCenter(double x, double y, double z);
  void setCutPlane(const ZPoint &center, const ZPoint &v1, const ZPoint &v2);
  void setCutPlane(const ZAffinePlane &plane);
  void setCutPlane(neutu::EAxis sliceAxis, const ZPoint &cutCenter);
  void setCutPlane(neutu::EAxis sliceAxis);

  /*!
   * \brief Set cut center and match the anchor point
   *
   * Set the cut center \a pt and map it to (\a ca, \b cb) in the canvas space.
   */
  void setCutCenter(const ZPoint &pt, double ca, double db);

  void setModelViewTransform(const ZModelViewTransform &t);
  void setModelViewTransform(const ZAffinePlane &cutPlane);
  void setModelViewTransform(neutu::EAxis sliceAxis, double cutDepth);
  void setModelViewTransform(neutu::EAxis sliceAxis, const ZPoint &cutCenter);
  void setViewCanvasTransform(double dx, double dy, double s);

  /*!
   * \brief Zoom to a certain point
   *
   * The new transform sets the cut center to (\a x, \a y, \a z) and scale to
   * \a s.
   */
  void zoomTo(double x, double y, double z, double s);

  /*!
   * \brief Zoom to a rectangle
   *
   * Maximum zoom to contain the view space rectangle
   * (\a u0, \a v0, \a u1, \a v1) in the target region
   * (0, 0, \a targetWidth, \a targetHeight).
   */
  void zoomToViewRect(double u0, double v0, double u1, double v1,
                  double targetWidth, double targetHeight);

  void zoomToViewRect(
      double srcWidth, double srcHeight,
      double targetWidth, double targetHeight);

  /*!
   * \brief Zoom to a rectangle
   *
   * Maximum zoom to contain the canvas space rectangle
   * (\a a0, \a b0, \a a1, \a b1) in the target region
   * (0, 0, \a targetWidth, \a targetHeight).
   */
  void zoomToCanvasRect(double a0, double b0, double a1, double b1,
                        double targetWidth, double targetHeight);

  void addScale(double ds);
  void setScaleFixingCanvasMapped(double s, double a, double b);

  void moveCutDepth(double z);
  void setCutDepth(const ZPoint &startPlane, double d);
  double getCutDepth(const ZPoint &startPlane) const;

  /*!
   * \brief Fit model range in a canvas
   *
   * It does not move the cut depth.
   */
  void fitModelRange(const ZIntCuboid &modelRange, int dstWidth, int dstHeight);

  /*!
   * \brief Translate the transform to match point mapping
   *
   * The point (\a x, \a y, \a z) is mapped to (\a a, \a b, \a n) after moving,
   * which only change translation of the transform.
   */
  void translateModelViewTransform(
      double x, double y, double z, double a, double b, double n);

  /*!
   * \brief Translate the transform to match point mapping
   *
   * Change translation without moving the cut depth. To move the point to the
   * current cut plane, use
   * \a translateModelViewTransform(\a x, \a y, \a, z, \a a, \a b, 0).
   */
  void translateModelViewTransform(
      double x, double y, double z, double a, double b);

  void translateModelViewTransform(const ZPoint &pt, double a, double b);

  enum class EPointMatchPolicy {
    CANVAS_TO_CANVAS, MODEL_To_CANVAS
  };

  void translateModelViewTransform(
      const ZPoint &src, const ZPoint &dst, EPointMatchPolicy policy);

  /*!
   * \brief Adjust the transformation to match the anchor point
   */
  void canvasAdjust(
      int width, int height, double viewAnchorX, double viewAnchorY);

  neutu::geom2d::Point getAnchor() const;
  /*!
   * \brief Set anchor point
   *
   * It does not change the cut center.
   */
  void setAnchor(double a, double b);
  void setAnchor(const neutu::geom2d::Point &pt);

  /*!
   * \brief Set the anchor point without modifying the actual transform
   */
  void moveAnchorTo(double a, double b);

  void translateViewCanvas(double u, double v, double a, double b);

  /*!
   * \brief Transform a canvas point back to model space
   */
  ZPoint inverseTransform(double a, double b, double n = 0) const;
  ZPoint inverseTransform(const ZPoint &pt) const;
  ZPoint inverseTransform(
      const ZPoint &pt, neutu::data3d::ESpace src, neutu::data3d::ESpace dst) const;

  ZPoint transform(const ZPoint &pt) const;
  ZPoint transform(double x, double y, double z) const;

  ZPoint transform(
      const ZPoint &pt, neutu::data3d::ESpace src, neutu::data3d::ESpace dst) const;

  ZAffineRect inverseTransformRect(double x0, double y0, double x1, double y1) const;

  ZPoint transformBoxSize(const ZPoint &dim) const;

  ZCuboid getViewBox(const ZIntCuboid modelBox) const;

  /*!
   * \brief Get view space viewport
   */
  neutu::geom2d::Rectangle getViewportV(
      int canvasWidth, int canvasHeight) const;

  ZJsonObject toJsonObject() const;

  bool operator== (const ZSliceViewTransform &t) const;
  bool operator!= (const ZSliceViewTransform &t) const;

  friend std::ostream& operator << (
      std::ostream& stream, const ZSliceViewTransform &t);

private:
  ZModelViewTransform m_modelViewTransform;
  ZViewPlaneTransform m_viewCanvasTransform;
};

#endif // ZSLICEVIEWTRANFORM_H
