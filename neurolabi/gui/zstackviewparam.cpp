#include "zstackviewparam.h"

#include <cmath>
#include <sstream>

#include "common/math.h"
#include "geometry/zgeometry.h"
#include "geometry/zaffineplane.h"
#include "geometry/zaffinerect.h"
#include "geometry/zcuboid.h"

#include "zarbsliceviewparam.h"
#include "zjsonobject.h"

ZStackViewParam::ZStackViewParam()
{
}

ZStackViewParam::ZStackViewParam(
    const ZSliceViewTransform &t, int width, int height,
    neutu::data3d::ESpace sizeSpace)
{
  set(t, width, height, sizeSpace);
}

void ZStackViewParam::set(
    const ZSliceViewTransform &t, int width, int height,
    neutu::data3d::ESpace sizeSpace)
{
  m_transform = t;
  setViewport(width, height, sizeSpace);
}

void ZStackViewParam::setTransform(const ZSliceViewTransform &t)
{
  m_transform = t;
}

void ZStackViewParam::setViewport(
    int width, int height, neutu::data3d::ESpace sizeSpace)
{
  m_viewportSize.set(width, height, sizeSpace);
}

ZAffineRect ZStackViewParam::getCutRect() const
{
  return m_transform.getCutRect(
        getWidth(m_viewportSize.m_space), getHeight(m_viewportSize.m_space),
        m_viewportSize.m_space);
}

ZAffineRect ZStackViewParam::getIntCutRect() const
{
  ZAffineRect rect = getCutRect();
  ZPoint center = rect.getCenter();
  rect.setCenter(center.roundToIntPoint().toPoint());
  int width = neutu::iround(rect.getWidth());
  int height = neutu::iround(rect.getHeight());
  if (!center.hasIntCoord()) {
    if (width > 0) {
      width += 2;
    }
    if (height > 0) {
      height += 2;
    }
  }
  rect.setSize(width, height);

  return rect;
//  return getDiscretized().getCutRect();
}

namespace {

ZAffineRect& empty_int_rect(ZAffineRect &rect)
{
  rect.setCenter(rect.getCenter().rounded());
  rect.setSize(0, 0);
  return rect;
}

}

namespace {

ZAffineRect adjust_rect(ZAffineRect rect, int zoom)
{
  int scale = pow(2, zoom + 1);
  ZPoint minCorner = rect.getMinCorner();

  int u0 = 0;
  int v0 = 0;

  if (rect.getV1() == ZPoint(1, 0, 0) || rect.getV1() == ZPoint(0, 1, 0) ||
      rect.getV1() == ZPoint(0, 0, 1)) {
    u0 = int(minCorner.dot(rect.getV1()));
  }

  if (rect.getV2() == ZPoint(1, 0, 0) || rect.getV2() == ZPoint(0, 1, 0) ||
      rect.getV2() == ZPoint(0, 0, 1)) {
    v0 = int(minCorner.dot(rect.getV2()));
  }

  int du = u0 % scale;
  int dv = v0 % scale;
  if (du > 0 || dv > 0) {
    rect.setSize(rect.getWidth() + du + du, rect.getHeight() + dv + dv);
  }

  int width = int(rect.getWidth());
  int height = int(rect.getHeight());
  if (width % scale > 0) {
    width += scale - width % scale;
  }
  if (height % scale > 0) {
    height += scale - height % scale;
  }
  rect.setSizeWithMinCornerFixed(width, height);

  return rect;
}

}

ZAffineRect ZStackViewParam::getIntCutRect(const ZIntCuboid &modelRange) const
{
  if (modelRange.isEmpty()) {
    return getIntCutRect();
  }

  ZAffineRect rect = getCutRect();

  if (!m_isViewportOpen) {
    return empty_int_rect(rect);
  }

  ZCuboid viewBox = m_transform.getViewBox(modelRange);
//  ZIntPoint newCenter = rect.getCenter().roundToIntPoint();

  double halfWidth = rect.getWidth() * 0.5;
  double halfHeight = rect.getHeight() * 0.5;

  double u0 = std::max(-halfWidth, viewBox.getMinCorner().getX());
  double u1 = std::min(halfWidth, viewBox.getMaxCorner().getX());
  double v0 = std::max(-halfHeight, viewBox.getMinCorner().getY());
  double v1 = std::min(halfHeight, viewBox.getMaxCorner().getY());

  if (u0 >= u1 || v0 >= v1) {
    return empty_int_rect(rect);
  }

  if (getSliceAxis() == neutu::EAxis::ARB) {
    std::pair<int, int> ur = zgeom::ToIntRange(u0, u1);
    std::pair<int, int> vr = zgeom::ToIntRange(v0, v1);

    //Use the left int center to be consistent with lowtis
    int width = ur.second - ur.first + 1;
    int height = vr.second - vr.first + 1;
    int du = ur.first + width / 2;
    int dv = vr.first + height / 2;
    ZPoint adjustedCenter =
        rect.getCenter() + m_transform.getCutPlane().getV1() * du +
        m_transform.getCutPlane().getV2() * dv;
    if (!adjustedCenter.hasIntCoord()) {
      width += 2;
      height += 2;
    }
    rect.setCenter(adjustedCenter.roundToIntPoint().toPoint());
    rect.setSize(width, height);
  } else {
//    ZIntPoint newCenter = rect.getCenter().roundToIntPoint();
    ZPoint viewCenter = rect.getCenter();
    ZIntPoint viewCenterInt =viewCenter.roundToIntPoint();
    ZIntCuboid viewBoxInt = modelRange - viewCenterInt;
    viewBoxInt.shiftSliceAxis(getSliceAxis());
    viewCenterInt.shiftSliceAxis(getSliceAxis());

    viewCenter.shiftSliceAxis(getSliceAxis());
    if (std::floor(viewCenter.getX()) != viewCenter.getX()) {
      u0 -= 0.5;
      u1 += 0.5;
    }
    if (std::floor(viewCenter.getY()) != viewCenter.getY()) {
      v0 -= 0.5;
      v1 += 0.5;
    }
    std::pair<int, int> ur = zgeom::ToIntRange(u0, u1);
    std::pair<int, int> vr = zgeom::ToIntRange(v0, v1);
//    ZIntCuboid viewBoxInt = viewBox.toIntCuboid();
    if (ur.first < viewBoxInt.getMinX()) {
      ur.first = viewBoxInt.getMinX();
    }
    if (ur.second > viewBoxInt.getMaxX()) {
      ur.second = viewBoxInt.getMaxX();
    }
    if (vr.first < viewBoxInt.getMinY()) {
      vr.first = viewBoxInt.getMinY();
    }
    if (vr.second > viewBoxInt.getMaxY()) {
      vr.second = viewBoxInt.getMaxY();
    }
    ZIntPoint newCenter = viewCenterInt;
    int width = ur.second - ur.first + 1;
    int height = vr.second - vr.first + 1;
    int du = ur.first + width / 2;
    int dv = vr.first + height / 2;
    newCenter += ZIntPoint(du, dv, 0);
    newCenter.shiftSliceAxisInverse(getSliceAxis());
    rect.setCenter(newCenter.toPoint());
    rect.setSize(width, height);
  }

  rect = adjust_rect(rect, getZoomLevel());

  return rect;
}

void ZStackViewParam::closeViewPort()
{
  m_isViewportOpen = false;
}

void ZStackViewParam::openViewPort()
{
  m_isViewportOpen = true;
}

/*
ZStackViewParam::ZStackViewParam(neutu::ECoordinateSystem coordSys)
{
  init(coordSys);
}

void ZStackViewParam::init(neutu::ECoordinateSystem coordSys)
{
  m_z = 0;
  m_coordSys = coordSys;
  m_action = neutu::View::EExploreAction::EXPLORE_UNKNOWN;
  m_fixingZ = false;
  m_sliceAxis = neutu::EAxis::Z;
}
*/

/*
QRectF ZStackViewParam::getProjRect() const
{
  return m_viewProj.getProjRect();
}

QRect ZStackViewParam::getViewPort() const
{
  return m_viewProj.getViewPort();
}

void ZStackViewParam::setZ(int z)
{
  m_z = z;
}

int ZStackViewParam::getSliceIndex() const
{
  return m_z - m_z0;
}

void ZStackViewParam::setSliceIndex(int index)
{
  m_z = index + m_z0;
}

void ZStackViewParam::setViewProj(const ZViewProj &vp)
{
  m_viewProj = vp;
}


void ZStackViewParam::setViewPort(const QRect &rect)
{
  m_viewProj.setViewPort(rect);
}

void ZStackViewParam::setViewPort(const QRect &rect, int z)
{
  setViewPort(rect);
  setZ(z);
}


void ZStackViewParam::closeViewPort()
{
  m_viewProj.closeViewPort();
}

void ZStackViewParam::openViewPort()
{
  m_viewProj.openViewPort();
}

void ZStackViewParam::setWidgetRect(const QRect &rect)
{
  m_viewProj.setWidgetRect(rect);
}

void ZStackViewParam::setCanvasRect(const QRect &rect)
{
  m_viewProj.setCanvasRect(rect);
}

void ZStackViewParam::setViewPort(double x0, double y0, double x1, double y1)
{
  QRect viewPort;
  viewPort.setTopLeft(QPoint(x0, y0));
  viewPort.setBottomRight(QPoint(x1, y1));

  setViewPort(viewPort);
}
*/

#if 0
void ZStackViewParam::setProjRect(const QRectF &rect)
{
  m_projRect = rect;
}
#endif

/*
void ZStackViewParam::setExploreAction(neutu::View::EExploreAction action)
{
  m_action = action;
}
*/

bool ZStackViewParam::operator ==(const ZStackViewParam &param) const
{
  return m_transform == param.m_transform &&
      m_viewportSize.m_width == param.m_viewportSize.m_width &&
      m_viewportSize.m_height == param.m_viewportSize.m_height &&
      m_viewportSize.m_space == param.m_viewportSize.m_space;
}

bool ZStackViewParam::operator !=(const ZStackViewParam &param) const
{
  return !(*this == param);
}

bool ZStackViewParam::contains(const ZStackViewParam &param) const
{
  if (param.isViewportEmpty()) {
    return m_transform.getCutPlane().contains(
          param.m_transform.getCutPlane(), 0.5);
  } else if (isViewportEmpty()) {
    return false;
  }

  return getCutRect().contains(param.getCutRect(), 0.5);
  /*
  if (m_transform.getCutPlane().onSamePlane(param.m_transform.getCutPlane())) {
    ZAffineRect rect2 = param.getCutRect();
    double canvasWidth = getWidth(neutu::data3d::ESpace::CANVAS);
    double canvasHeight = getHeight(neutu::data3d::ESpace::CANVAS);
    for (int i = 0; i < 4; ++i) {
      ZPoint corner = rect2.getCorner(i);
      ZPoint canvasCorner = m_transform.transform(
            corner, neutu::data3d::ESpace::MODEL, neutu::data3d::ESpace::CANVAS);
      if (canvasCorner.getX() < 0.0 || canvasCorner.getX() > canvasWidth ||
          canvasCorner.getY() < 0.0 || canvasCorner.getY() > canvasHeight) {
        return false;
      }
    }

    return true;
  }

  return false;
  */
}

bool ZStackViewParam::contains(double x, double y, double z) const
{
  return contains(ZPoint(x, y, z));
}

bool ZStackViewParam::contains(const ZPoint &pt) const
{
  return getCutRect().contains(pt, 0.5);
}

/*
bool ZStackViewParam::containsViewport(const ZStackViewParam &param) const
{
  return getViewPort().contains(param.getViewPort());
}
*/

#if 0
bool ZStackViewParam::contains(double x, double y, double z) const
{
  ZPoint pt = m_transform.transform(
        ZPoint(x, y, z),
        neutu::data3d::ESpace::MODEL, neutu::data3d::ESpace::CANVAS);

  return (pt.getZ() < ZPoint::MIN_DIST) &&
      (pt.getX() >= 0.0) && (pt.getX() <= m_viewportSize.m_width) &&
      (pt.getY() >= 0.0) && (pt.getY() <= m_viewportSize.m_height);

  /*
  zgeom::shiftSliceAxis(x, y, z, getSliceAxis());

  if (z == m_z) {
    return getViewPort().contains(x, y);
  }

  return false;
  */
}
#endif

void ZStackViewParam::setSize(
    int width, int height, neutu::data3d::ESpace sizeSpace)
{
  m_viewportSize.set(width, height, sizeSpace);
}

namespace {
double get_dim(double d, double scale, neutu::data3d::ESpace sourceSpace,
               neutu::data3d::ESpace targetSpace, bool viewOpen)
{
  if (viewOpen == false  || scale <= 0.0) {
    return 0.0;
  }

  if (sourceSpace != targetSpace) {
    switch(targetSpace) {
    case neutu::data3d::ESpace::CANVAS:
      d *= scale;
      break;
    case neutu::data3d::ESpace::VIEW:
    case neutu::data3d::ESpace::MODEL:
      if (sourceSpace == neutu::data3d::ESpace::CANVAS) {
        d /= scale;
      }
      break;
    }
  }

  return d;
}

}

double ZStackViewParam::getWidth(neutu::data3d::ESpace space) const
{
  return get_dim(
        m_viewportSize.m_width, m_transform.getScale(),
        m_viewportSize.m_space, space, m_isViewportOpen);
}

double ZStackViewParam::getHeight(neutu::data3d::ESpace space) const
{
  return get_dim(
        m_viewportSize.m_height, m_transform.getScale(),
        m_viewportSize.m_space, space, m_isViewportOpen);
}

int ZStackViewParam::getIntWidth(neutu::data3d::ESpace space) const
{
  return neutu::iceil(getWidth(space));
}

int ZStackViewParam::getIntHeight(neutu::data3d::ESpace space) const
{
  return neutu::iceil(getHeight(space));
}

ZPoint ZStackViewParam::getCutCenter() const
{
  return m_transform.getCutCenter();
}

void ZStackViewParam::setCutCenter(const ZIntPoint &pt)
{
  m_transform.setCutCenter(pt.toPoint());
}

void ZStackViewParam::setCutDepth(const ZPoint &startPlane, double d)
{
  m_transform.setCutDepth(startPlane, d);
}

void ZStackViewParam::moveCutDepth(double d)
{
  m_transform.moveCutDepth(d);
}

double ZStackViewParam::getCutDepth(const ZPoint &startPlane) const
{
  return m_transform.getCutDepth(startPlane);
}

ZArbSliceViewParam ZStackViewParam::toArbSliceViewParam() const
{
  ZAffineRect rect = getIntCutRect();

  ZArbSliceViewParam param;
  param.setCenter(rect.getCenter().roundToIntPoint());
  param.setPlane(rect.getV1(), rect.getV1());
  param.setSize(rect.getWidth(), rect.getHeight());

  /*
  int width = getIntWidth(neutu::data3d::ESpace::MODEL);
  int height = getIntHeight(neutu::data3d::ESpace::MODEL);
  if (!getCutCenter().hasIntCoord()) {
    ++width;
    ++height;
  }
  param.setCenter(getCutCenter().toIntPoint());
  param.setPlane(
        m_transform.getCutPlane().getV1(), m_transform.getCutPlane().getV2());
  param.setSize(width, height);
  */

  return param;
}

/*
void ZStackViewParam::discretizeModel()
{
  if (m_viewportSize.m_space == neutu::data3d::ESpace::CANVAS) {
    m_viewportSize.set(
          getIntWidth(neutu::data3d::ESpace::MODEL),
          getIntHeight(neutu::data3d::ESpace::MODEL),
          neutu::data3d::ESpace::MODEL);
  }

  if (!getCutCenter().hasIntCoord()) {
    m_viewportSize.m_width += 2;
    m_viewportSize.m_height += 2;
  }

  setCutCenter(getCutCenter().toIntPoint());
}

ZStackViewParam ZStackViewParam::getDiscretized() const
{
  ZStackViewParam param = *this;
  param.discretizeModel();
  return param;
}
*/

/*
void ZStackViewParam::resize(int width, int height)
{
  QRect viewPort = m_viewProj.getViewPort();
  QPoint oldCenter = getViewPort().center();
  viewPort.setSize(QSize(width, height));
  viewPort.moveCenter(oldCenter);
  m_viewProj.setViewPort(viewPort);
}
*/

bool ZStackViewParam::isViewportEmpty() const
{
  return !m_isViewportOpen || m_transform.getScale() <= 0 ||
      m_viewportSize.m_width <= 0 || m_viewportSize.m_height <= 0;
}

bool ZStackViewParam::isValid() const
{
  return !isViewportEmpty();
}

void ZStackViewParam::invalidate()
{
  m_viewportSize.set(0, 0);
}

double ZStackViewParam::getArea(neutu::data3d::ESpace space) const
{
  return getWidth(space) * getHeight(space);
//  return getCutRect().getArea();
//  return size_t(getViewPort().width()) * size_t(getViewPort().height());
}

void ZStackViewParam::setSliceAxis(neutu::EAxis sliceAxis)
{
  m_transform.setCutPlane(sliceAxis);
}

neutu::EAxis ZStackViewParam::getSliceAxis() const
{
  return m_transform.getSliceAxis();
}

int ZStackViewParam::getZoomLevel() const
{
//  int zoom = std::round(std::log(1.0 / getZoomRatio()) / std::log(2.0) ) -1;

//  if (zoom < 0) {
//    zoom = 0;
//  }

//  int scale = pow(2, zoom);
  int zoom = 0;
  int scale = 1;
  while (getArea(neutu::data3d::ESpace::MODEL) / scale / scale > 512 * 512) {
    zoom += 1;
    scale = pow(2, zoom);
  }
  return zoom;
}

int ZStackViewParam::getZoomLevel(int maxLevel) const
{
  int zoom = getZoomLevel();

  if (zoom > maxLevel) {
    zoom = maxLevel;
  }

  return zoom;
}
/*
ZArbSliceViewParam ZStackViewParam::getSliceViewParam() const
{
  ZArbSliceViewParam param;
  param.setCenter(m_center);
  param.setPlane(m_v1, m_v2);
  QRect viewPort = getViewPort();
  param.setSize(viewPort.width(), viewPort.height());

  return param;
}

void ZStackViewParam::setArbSliceCenter(const ZIntPoint &pt)
{
  m_center = pt;
}

void ZStackViewParam::setArbSlicePlane(const ZPoint &v1, const ZPoint &v2)
{
  m_v1 = v1;
  m_v2 = v2;
}

void ZStackViewParam::setArbSliceView(const ZArbSliceViewParam &param)
{
  setArbSliceCenter(param.getCenter());
  setArbSlicePlane(param.getPlaneV1(), param.getPlaneV2());
  if (!param.getViewPort().isEmpty()) {
    setViewPort(param.getViewPort());
  }
}

void ZStackViewParam::moveSlice(int step)
{
  m_z += step;
  if (m_sliceAxis == neutu::EAxis::ARB) {
    ZPoint dp = m_v1.cross(m_v2) * step;
    m_center += dp.toIntPoint();
  }
}

ZAffineRect ZStackViewParam::getSliceRect() const
{
  ZAffinePlane plane = getArbSlicePlane();
  ZAffineRect rect;
  rect.setPlane(plane.getV1(), plane.getV2());
  ZIntPoint center = m_center;
  QRect viewPort = getViewPort();
  if (m_sliceAxis != neutu::EAxis::ARB) {
    center.set(viewPort.center().x(), viewPort.center().y(), m_z);
    center.shiftSliceAxis(m_sliceAxis);
  }
  rect.setCenter(center.toPoint());
  rect.setSize(viewPort.width(), viewPort.height());

  return rect;
}

ZAffinePlane ZStackViewParam::getArbSlicePlane() const
{
  ZAffinePlane plane;
  switch (m_sliceAxis) {
  case neutu::EAxis::ARB:
    plane.set(m_center.toPoint(), m_v1, m_v2);
    break;
  case neutu::EAxis::Z:
    plane.set(ZPoint(0, 0, m_z), ZPoint(1, 0, 0), ZPoint(0, 1, 0));
    break;
  case neutu::EAxis::Y:
    plane.set(ZPoint(0, m_z, 0), ZPoint(0, 0, 1), ZPoint(1, 0, 0));
    break;
  case neutu::EAxis::X:
    plane.set(ZPoint(m_z, 0, 0), ZPoint(0, 1, 0), ZPoint(0, 0, 1));
    break;
  }

  return plane;
}
*/

double ZStackViewParam::getZoomRatio() const
{
  return m_transform.getScale();
//  return m_viewProj.getZoom();
}

bool ZStackViewParam::onSamePlane(const ZStackViewParam &param) const
{
  return m_transform.getCutPlane().onSamePlane(
        param.m_transform.getCutPlane());
  /*
  bool result = false;
  if (m_sliceAxis == param.m_sliceAxis) {
    if (m_sliceAxis == neutu::EAxis::ARB) {
      result = zgeom::IsSameAffinePlane(
            m_center.toPoint(), m_v1, m_v2,
            param.m_center.toPoint(), param.m_v1, param.m_v2);
    } else {
      result = (m_z == param.m_z);
    }
  }

  return result;
  */
}

/*
std::string ZStackViewParam::toString() const
{
  std::ostringstream stream;
  if (m_sliceAxis != neutu::EAxis::ARB) {
    stream << "Axis=" << neutu::EnumValue(m_sliceAxis) << "; ";
    QRect vp = getViewPort();
    stream << "(" << vp.x() << "," << vp.y() << ")"
           << vp.width() << "x" << vp.height() << "; z=" << getZ();
  }

  return stream.str();
}
*/

/*
namespace {
template<typename T1, typename T2>
void point_to_array(const T1 &pt, T2 *v)
{
  v[0] = pt.getX();
  v[1] = pt.getY();
  v[2] = pt.getZ();
}
}
*/

ZSliceViewTransform ZStackViewParam::getSliceViewTransform() const
{
  return m_transform;
}

void ZStackViewParam::setViewId(int id)
{
  m_viewId = id;
}

int ZStackViewParam::getViewId() const
{
  return m_viewId;
}

ZJsonObject ZStackViewParam::toJsonObject() const
{
  return m_transform.toJsonObject();

  /*
  ZJsonObject jsonObj = m_viewProj.toJsonObject();
  jsonObj.setEntry("axis", neutu::EnumValue(m_sliceAxis));
  jsonObj.setEntry("z", m_z);
  if (m_sliceAxis == neutu::EAxis::ARB) {
    int center[3];
    point_to_array(m_center, center);

    double v1[3], v2[3];
    point_to_array(m_v1, v1);
    point_to_array(m_v2, v2);

    jsonObj.setEntry("center", center, 3);
    jsonObj.setEntry("v1", v1, 3);
    jsonObj.setEntry("v2", v2, 3);
  }

  return jsonObj;
  */
}
