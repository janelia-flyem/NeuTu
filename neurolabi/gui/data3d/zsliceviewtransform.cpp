#include "zsliceviewtransform.h"

#include "geometry/zgeometry.h"
#include "geometry/zaffinerect.h"
#include "geometry/zcuboid.h"
#include "zjsonobject.h"

ZSliceViewTransform::ZSliceViewTransform()
{

}

ZSliceViewTransform::ZSliceViewTransform(
    const ZModelViewTransform &tmv, const ZViewPlaneTransform &tvc)
{
  m_modelViewTransform = tmv;
  m_viewCanvasTransform  = tvc;
}

neutu::EAxis ZSliceViewTransform::getSliceAxis() const
{
  return m_modelViewTransform.getSliceAxis();
}

void ZSliceViewTransform::setSliceAxis(neutu::EAxis axis)
{
  m_modelViewTransform.setCutPlane(axis);
}

ZPoint ZSliceViewTransform::getCutPlaneNormal() const
{
  return m_modelViewTransform.getCutPlaneNormal();
}

ZPoint ZSliceViewTransform::getCutCenter() const
{
  return m_modelViewTransform.getCutCenter();
}

ZAffinePlane ZSliceViewTransform::getCutPlane() const
{
  return m_modelViewTransform.getCutPlane();
}

ZPlane ZSliceViewTransform::getCutOrientation() const
{
  return m_modelViewTransform.getCutPlane().getPlane();
}

double ZSliceViewTransform::getScale() const
{
  return getViewCanvasTransform().getScale();
}

void ZSliceViewTransform::setRightHanded(bool r)
{
  m_modelViewTransform.setRightHanded(r);
}

bool ZSliceViewTransform::rightHanded() const
{
  return m_modelViewTransform.rightHanded();
}

ZAffineRect ZSliceViewTransform::getCutRect(
    double width, double height, neutu::data3d::ESpace sizeSpace) const
{
  double cx = width / 2.0;
  double cy = height / 2.0;
  if (sizeSpace != neutu::data3d::ESpace::CANVAS) {
    cx *= getScale();
    cy *= getScale();
  }

  ZPoint newCutCenter = inverseTransform(cx, cy);

  ZAffineRect rect;
  rect.setCenter(newCutCenter);
  rect.setPlane(m_modelViewTransform.getCutPlane().getV1(),
                m_modelViewTransform.getCutPlane().getV2());

  double modelWidth = width;
  double modelHeight = height;
  if (sizeSpace == neutu::data3d::ESpace::CANVAS) {
    modelWidth /= getScale();
    modelHeight /= getScale();
  }

  rect.setSize(modelWidth, modelHeight);

  return rect;
}

ZAffineRect ZSliceViewTransform::getCutRect(
    int canvasWidth, int canvasHeight) const
{
  return getCutRect(canvasWidth, canvasHeight, neutu::data3d::ESpace::CANVAS);
}

ZAffineRect ZSliceViewTransform::getIntCutRect(
    int width, int height, neutu::data3d::ESpace sizeSpace)
{
  ZAffineRect rect = getCutRect(width, height, sizeSpace);
  if (!getCutCenter().hasIntCoord()) {
    setCutCenter(getCutCenter().roundToIntPoint().toPoint());
    rect.setSize(rect.getWidth() + 1, rect.getHeight() + 1);
  }

  return rect;
}

void ZSliceViewTransform::setScale(double s)
{
  m_viewCanvasTransform.setScale(s);
}

void ZSliceViewTransform::incScale()
{
  m_viewCanvasTransform.incScale();
}

void ZSliceViewTransform::decScale()
{
  m_viewCanvasTransform.decScale();
}

double ZSliceViewTransform::getScaleStep() const
{
  return m_viewCanvasTransform.getScaleStep();
}

void ZSliceViewTransform::setMinScale(double s)
{
  m_viewCanvasTransform.setMinScale(s);
}

double ZSliceViewTransform::getMinScale() const
{
  return m_viewCanvasTransform.getMinScale();
}

double ZSliceViewTransform::getMaxScale() const
{
  return m_viewCanvasTransform.getMaxScale();
}

ZModelViewTransform ZSliceViewTransform::getModelViewTransform() const
{
  return m_modelViewTransform;
}

ZViewPlaneTransform ZSliceViewTransform::getViewCanvasTransform() const
{
  return m_viewCanvasTransform;
}

void ZSliceViewTransform::setModelViewTransform(const ZModelViewTransform &t)
{
  m_modelViewTransform = t;
}

void ZSliceViewTransform::setModelViewTransform(const ZAffinePlane &cutPlane)
{
  m_modelViewTransform.setCutPlane(cutPlane);
}

void ZSliceViewTransform::setModelViewTransform(
    neutu::EAxis sliceAxis, double cutDepth)
{
  m_modelViewTransform.setCutPlane(sliceAxis, cutDepth);
}

void ZSliceViewTransform::setModelViewTransform(
    neutu::EAxis sliceAxis, const ZPoint &cutCenter)
{
  m_modelViewTransform.setCutPlane(sliceAxis, cutCenter);
}

void ZSliceViewTransform::setCutPlane(neutu::EAxis sliceAxis)
{
  m_modelViewTransform.setCutPlane(sliceAxis);
}

void ZSliceViewTransform::setViewCanvasTransform(double dx, double dy, double s)
{
  m_viewCanvasTransform.set(dx, dy, s);
}

void ZSliceViewTransform::moveCutDepth(double z)
{
  m_modelViewTransform.moveCutDepth(z);
}

void ZSliceViewTransform::setCutDepth(const ZPoint &startPlane, double d)
{
  m_modelViewTransform.setCutDepth(startPlane, d);
}

double ZSliceViewTransform::getCutDepth(const ZPoint &startPlane) const
{
  return m_modelViewTransform.getCutDepth(startPlane);
}

void ZSliceViewTransform::translateModelViewTransform(
    const ZPoint &src, const ZPoint &dst, EPointMatchPolicy policy)
{
  switch (policy) {
  case EPointMatchPolicy::CANVAS_TO_CANVAS:
    translateModelViewTransform(
          inverseTransform(src), dst, EPointMatchPolicy::MODEL_To_CANVAS);
    break;
  case EPointMatchPolicy::MODEL_To_CANVAS:
    translateModelViewTransform(
          src.getX(), src.getY(), src.getZ(),
          dst.getX(), dst.getY(), dst.getZ());
    break;
  }
}

void ZSliceViewTransform::translateModelViewTransform(
    double x, double y, double z, double a, double b, double n)
{
  m_modelViewTransform.transform(&x, &y, &z);
  m_viewCanvasTransform.inverseTransform(&a, &b);
  m_modelViewTransform.translateCutCenterOnPlane(x - a, y - b);
  m_modelViewTransform.moveCutDepth(z - n);

  /*
  neutu::EAxis sliceAxis = getSliceAxis();
  switch(sliceAxis) {
  case neutu::EAxis::X:
  case neutu::EAxis::Y:
  case neutu::EAxis::Z:
    zgeom::shiftSliceAxis(x, y, z, sliceAxis);
    m_viewCanvasTransform.translateTransform(x, y, a, b);
    break;
   case neutu::EAxis::ARB:
    //Needs to move the world transform
    m_modelViewTransform.transform(&x, &y, &z);
    m_viewCanvasTransform.inverseTransform(&a, &b);
    m_modelViewTransform.translateCutCenterOnPlane(x - a, y - b);
    break;
  }

  moveCutDepth(z - n);
  */
}

void ZSliceViewTransform::translateModelViewTransform(
    double x, double y, double z, double a, double b)
{
  m_modelViewTransform.transform(&x, &y, &z);
  m_viewCanvasTransform.inverseTransform(&a, &b);
  m_modelViewTransform.translateCutCenterOnPlane(x - a, y - b);
  m_modelViewTransform.moveCutDepth(z);
  /*
  neutu::EAxis sliceAxis = getSliceAxis();
  switch(sliceAxis) {
  case neutu::EAxis::X:
  case neutu::EAxis::Y:
  case neutu::EAxis::Z:
    zgeom::shiftSliceAxis(x, y, z, sliceAxis);
    m_viewCanvasTransform.translateTransform(x, y, a, b);
    break;
   case neutu::EAxis::ARB:
    //Needs to move the world transform
    m_modelViewTransform.transform(&x, &y, &z);
    m_viewCanvasTransform.inverseTransform(&a, &b);
    m_modelViewTransform.translateCutCenterOnPlane(x - a, y - b);
    break;
  }
  */
}

void ZSliceViewTransform::translateModelViewTransform(
    const ZPoint &pt, double a, double b)
{
  translateModelViewTransform(pt.getX(), pt.getY(), pt.getZ(), a, b);
}

void ZSliceViewTransform::zoomTo(double x, double y, double z, double s)
{
  m_modelViewTransform.setCutCenter(x, y, z);
  m_viewCanvasTransform.setScaleFixingOriginal(s, 0, 0);
}

void ZSliceViewTransform::zoomToViewRect(
    double u0, double v0, double u1, double v1,
    double targetWidth, double targetHeight)
{
  double scale = std::min(targetWidth / (u1 - u0), targetHeight / (v1 - v0));
  m_viewCanvasTransform.setScale(scale);

  double cu = (u0 + u1) / 2.0;
  double cv = (v0 + v1) / 2.0;
  double cu2 = targetWidth / 2.0;
  double cv2 = targetHeight / 2.0;
  m_viewCanvasTransform.inverseTransform(&cu2, &cv2);

  m_modelViewTransform.translateCutCenterOnPlane(cu - cu2, cv - cv2);
}

void ZSliceViewTransform::zoomToViewRect(
    double srcWidth, double srcHeight, double targetWidth, double targetHeight)
{
  zoomToViewRect(
        -srcWidth / 2.0, -srcHeight / 2.0, srcWidth / 2.0, srcHeight / 2.0,
        targetWidth, targetHeight);
}

void ZSliceViewTransform::zoomToCanvasRect(
    double a0, double b0, double a1, double b1,
    double targetWidth, double targetHeight)
{
  m_viewCanvasTransform.inverseTransform(&a0, &b0);
  m_viewCanvasTransform.inverseTransform(&a1, &b1);
  zoomToViewRect(a0, b0, a1, b1, targetWidth, targetHeight);
}

void ZSliceViewTransform::addScale(double ds)
{
  m_viewCanvasTransform.setScaleFixingOriginal(
        0, 0, m_viewCanvasTransform.getScale() + ds);
}

void ZSliceViewTransform::rotate(double au, double av, double rad)
{
  m_modelViewTransform.rotate(au, av, rad);
}

void ZSliceViewTransform::setScaleFixingCanvasMapped(
    double s, double a, double b)
{
  ZPoint fixPoint = inverseTransform(a, b);
  m_viewCanvasTransform.setScale(s);
  translateModelViewTransform(fixPoint, a, b);
}

void ZSliceViewTransform::fitModelRange(
    const ZIntCuboid &modelRange, int dstWidth, int dstHeight)
{
  if (!modelRange.isEmpty()) {
    /*
    double srcWidth = modelRange.getWidth();
    double srcHeight = modelRange.getHeight();
    if (m_modelViewTransform.getSliceAxis() == neutu::EAxis::ARB) {
      srcHeight = srcWidth = modelRange.getDiagonalLength();
    }*/

    ZCuboid viewBox = getViewBox(modelRange);

    /*
    ZCuboid box = ZCuboid::FromIntCuboid(modelRange);
    for (int i = 0; i < 8; ++i) {
      ZPoint corner = box.getCorner(i);
      ZPoint pc = transform(corner, neutu::data3d::ESpace::MODEL,
                            neutu::data3d::ESpace::VIEW);
      if (i == 0) {
        viewBox.set(pc, pc);
      } else {
        viewBox.join(pc);
      }
    }
    */

    /*
    ZPoint dims = m_modelViewTransform.transformBoxSize(
          modelRange.getSize().toPoint());
          */

    double scale = std::min(
          dstWidth / viewBox.width(), dstHeight / viewBox.height());
    setScale(scale);
//    ZPoint center = modelRange.getExactCenter();
    setAnchor(dstWidth / 2.0, dstHeight / 2.0);
    ZPoint viewCenter = viewBox.getCenter();
    m_modelViewTransform.translateCutCenterOnPlane(
          viewCenter.getX(), viewCenter.getY());
//    translateModelViewTransform(center, dstWidth / 2.0, dstHeight / 2.0);
  }
}

void ZSliceViewTransform::canvasAdjust(
    int width, int height, double viewAnchorX, double viewAnchorY)
{
  setAnchor(width * viewAnchorX, height * viewAnchorY);
}

void ZSliceViewTransform::translateViewCanvas(
    double u, double v, double a, double b)
{
  m_viewCanvasTransform.translateTransform(u, v, a, b);
}

ZPoint ZSliceViewTransform::transform(const ZPoint &pt) const
{
  return m_viewCanvasTransform.transform(
        m_modelViewTransform.transform(pt));
}

ZPoint ZSliceViewTransform::transform(double x, double y, double z) const
{
  return transform(ZPoint(x, y, z));
}

ZPoint ZSliceViewTransform::transform(
    const ZPoint &pt, neutu::data3d::ESpace src, neutu::data3d::ESpace dst) const
{
  if (src != dst) {
    switch (src) {
    case neutu::data3d::ESpace::MODEL:
      switch (dst) {
      case neutu::data3d::ESpace::VIEW: // model -> view
        return m_modelViewTransform.transform(pt);
      case neutu::data3d::ESpace::CANVAS: // model -> canvas
        return transform(pt);
      default:
        break;
      }
    case neutu::data3d::ESpace::VIEW:
      switch (dst) {
      case neutu::data3d::ESpace::MODEL: // view -> model
        return m_modelViewTransform.inverseTransform(
              pt.getX(), pt.getY(), pt.getZ());
      case neutu::data3d::ESpace::CANVAS: // view -> canvas
        return m_viewCanvasTransform.transform(pt);
      default:
        break;
      }
      break;
    case neutu::data3d::ESpace::CANVAS:
      switch (dst) {
      case neutu::data3d::ESpace::MODEL: // canvas -> model
        return inverseTransform(pt);
      case neutu::data3d::ESpace::VIEW: // canvas -> view
        return m_viewCanvasTransform.inverseTransform(pt);
      default:
        break;
      }
      break;
    }
  }

  return pt;
}

ZPoint ZSliceViewTransform::inverseTransform(double a, double b, double n) const
{
  m_viewCanvasTransform.inverseTransform(&a, &b);

  return m_modelViewTransform.inverseTransform(a, b, n);
}

ZPoint ZSliceViewTransform::inverseTransform(const ZPoint &pt) const
{
  return inverseTransform(pt.getX(), pt.getY(), pt.getZ());
}

ZPoint ZSliceViewTransform::inverseTransform(
    const ZPoint &pt, neutu::data3d::ESpace src, neutu::data3d::ESpace dst) const
{
  return transform(pt, dst, src);
}

ZPoint ZSliceViewTransform::transformBoxSize(const ZPoint &dim) const
{
  ZPoint newDim = m_modelViewTransform.transformBoxSize(dim);
  newDim.setX(newDim.getX() * getScale());
  newDim.setY(newDim.getY() * getScale());

  return newDim;
}

bool ZSliceViewTransform::onSamePlane(const ZSliceViewTransform &t) const
{
  return m_modelViewTransform.onSamePlane(t.m_modelViewTransform);
}

bool ZSliceViewTransform::hasSamePlane(const ZSliceViewTransform &t) const
{
  return m_modelViewTransform.hasSamePlane(t.m_modelViewTransform);
}

ZCuboid ZSliceViewTransform::getViewBox(const ZIntCuboid modelBox) const
{
  ZCuboid viewBox;

  ZCuboid box = ZCuboid::FromIntCuboid(modelBox);
  for (int i = 0; i < 8; ++i) {
    ZPoint corner = box.getCorner(i);
    ZPoint pc = transform(corner, neutu::data3d::ESpace::MODEL,
                          neutu::data3d::ESpace::VIEW);
    if (i == 0) {
      viewBox.set(pc, pc);
    } else {
      viewBox.join(pc);
    }
  }

  return viewBox;
}

neutu::geom2d::Rectangle ZSliceViewTransform::getViewportV(
    int canvasWidth, int canvasHeight) const
{
  return m_viewCanvasTransform.inverseTransform(
        neutu::geom2d::Rectangle(0, 0, canvasWidth, canvasHeight));
}

ZAffineRect ZSliceViewTransform::inverseTransformRect(
    double x0, double y0, double x1, double y1) const
{
  ZAffineRect rect;
  rect.setPlane(m_modelViewTransform.getCutPlane());
  rect.setSize((x1 - x0) / getScale(), (y1 - y0) / getScale());
  rect.setCenter(inverseTransform((x0 + y0) / 2.0, (x1 + y1) / 2.0));

  return rect;
}

void ZSliceViewTransform::setCutCenter(const ZPoint &pt)
{
  m_modelViewTransform.setCutCenter(pt);
}

void ZSliceViewTransform::setCutCenter(double x, double y, double z)
{
  m_modelViewTransform.setCutCenter(x, y, z);
}

void ZSliceViewTransform::setCutCenter(const ZPoint &pt, double ca, double cb)
{
  m_modelViewTransform.setCutCenter(pt);
  m_viewCanvasTransform.setOffset(ca, cb);
}

void ZSliceViewTransform::setCutPlane(
    const ZPoint &center, const ZPoint &v1, const ZPoint &v2)
{
  m_modelViewTransform.setCutPlane(center, v1, v2);
}

void ZSliceViewTransform::setCutPlane(const ZPoint &v1, const ZPoint &v2)
{
  m_modelViewTransform.setCutPlane(v1, v2);
}

void ZSliceViewTransform::setCutPlane(const ZPlane &p)
{
  setCutPlane(p.getV1(), p.getV2());
}

void ZSliceViewTransform::setCutPlane(const ZAffinePlane &plane)
{
  m_modelViewTransform.setCutPlane(plane);
}

void ZSliceViewTransform::setCutPlane(
    neutu::EAxis sliceAxis, const ZPoint &cutCenter)
{
  m_modelViewTransform.setCutPlane(sliceAxis, cutCenter);
}

neutu::geom2d::Point ZSliceViewTransform::getAnchor() const
{
  return neutu::geom2d::Point(
        m_viewCanvasTransform.getTx(), m_viewCanvasTransform.getTy());
}

void ZSliceViewTransform::setAnchor(double a, double b)
{
  m_viewCanvasTransform.setOffset(a, b);
}

void ZSliceViewTransform::setAnchor(const neutu::geom2d::Point &pt)
{
  setAnchor(pt.getX(), pt.getY());
}

void ZSliceViewTransform::moveAnchorTo(double a, double b)
{
  if (getAnchor() != neutu::geom2d::Point(a, b)) {
    ZPoint newCutCenter = inverseTransform(a, b);
    m_viewCanvasTransform.setOffset(a, b);
    setCutCenter(newCutCenter);
  }
}

ZJsonObject ZSliceViewTransform::toJsonObject() const
{
  ZJsonObject obj;

  obj.setEntry("model->view", m_modelViewTransform.toJsonObject());
  obj.setEntry("view->canvas", m_viewCanvasTransform.toJsonObject());

  return obj;
}

bool ZSliceViewTransform::operator==(const ZSliceViewTransform &t) const
{
  return m_modelViewTransform == t.m_modelViewTransform &&
      m_viewCanvasTransform == t.m_viewCanvasTransform;
}

bool ZSliceViewTransform::operator!=(const ZSliceViewTransform &t) const
{
  return !(*this == t);
}

std::ostream& operator << (
      std::ostream& stream, const ZSliceViewTransform &t)
{
  stream << t.m_modelViewTransform << " -> " << t.m_viewCanvasTransform;
  return stream;
}

void ZSliceViewTransform::copyWithoutOrientation(const ZSliceViewTransform &t)
{
  m_modelViewTransform.copyWithoutOrientation(t.m_modelViewTransform);
  m_viewCanvasTransform = t.m_viewCanvasTransform;
}


