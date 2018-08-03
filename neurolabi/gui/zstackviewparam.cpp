#include "zstackviewparam.h"

#include <cmath>

#include "geometry/zgeometry.h"
#include "tz_math.h"
#include "zarbsliceviewparam.h"

ZStackViewParam::ZStackViewParam()
{
  init(neutube::COORD_RAW_STACK);
}

ZStackViewParam::ZStackViewParam(neutube::ECoordinateSystem coordSys)
{
  init(coordSys);
}

void ZStackViewParam::init(neutube::ECoordinateSystem coordSys)
{
  m_z = 0;
  m_coordSys = coordSys;
  m_action = neutube::View::EXPLORE_UNKNOWN;
  m_fixingZ = false;
  m_sliceAxis = neutube::Z_AXIS;
}

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

#if 0
void ZStackViewParam::setProjRect(const QRectF &rect)
{
  m_projRect = rect;
}
#endif

void ZStackViewParam::setExploreAction(neutube::View::EExploreAction action)
{
  m_action = action;
}

bool ZStackViewParam::operator ==(const ZStackViewParam &param) const
{
  if (getSliceAxis() != param.getSliceAxis()) {
    return false;
  }

  if (getSliceAxis() == neutube::A_AXIS) {
    if (getSliceViewParam() != param.getSliceViewParam()) {
      return false;
    }
  }

  return m_z == param.m_z && m_coordSys == param.m_coordSys &&
      getViewPort() == param.getViewPort();
}

bool ZStackViewParam::operator !=(const ZStackViewParam &param) const
{
  return !(*this == param);
}

bool ZStackViewParam::contains(const ZStackViewParam &param) const
{
  if (getSliceAxis() == param.getSliceAxis()) {
    if (getSliceAxis() == neutube::A_AXIS) {
      return getSliceViewParam().contains(param.getSliceViewParam());
    } else if (m_z == param.m_z) {
      if (param.getViewPort().isEmpty()) {
        return true;
      } else {
        return getViewPort().contains(param.getViewPort());
      }
    }
  }

  return false;
}

bool ZStackViewParam::containsViewport(const ZStackViewParam &param) const
{
  return getViewPort().contains(param.getViewPort());
}

bool ZStackViewParam::contains(int x, int y, int z)
{
  zgeom::shiftSliceAxis(x, y, z, getSliceAxis());

  if (z == m_z) {
    return getViewPort().contains(x, y);
  }

  return false;
}


void ZStackViewParam::resize(int width, int height)
{
  QRect viewPort = m_viewProj.getViewPort();
  QPoint oldCenter = getViewPort().center();
  viewPort.setSize(QSize(width, height));
  viewPort.moveCenter(oldCenter);
  m_viewProj.setViewPort(viewPort);
}

bool ZStackViewParam::isValid() const
{
  return m_viewProj.isValid();
}

void ZStackViewParam::invalidate()
{
  m_viewProj.setZoom(0);
}

size_t ZStackViewParam::getArea() const
{
  return size_t(getViewPort().width()) * size_t(getViewPort().height());
}

void ZStackViewParam::setSliceAxis(neutube::EAxis sliceAxis)
{
  m_sliceAxis = sliceAxis;
}

neutube::EAxis ZStackViewParam::getSliceAxis() const
{
  return m_sliceAxis;
}

int ZStackViewParam::getZoomLevel() const
{
  int zoom = std::round(std::log(1.0 / getZoomRatio()) / std::log(2.0) ) -1;

  if (zoom < 0) {
    zoom = 0;
  }

  int scale = pow(2, zoom);
  while (getViewPort().width() * getViewPort().height() /
      scale / scale > 1024 * 1024) {
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
  if (m_sliceAxis == neutube::A_AXIS) {
    ZPoint dp = m_v1.cross(m_v2) * step;
    m_center += dp.toIntPoint();
  }
}

double ZStackViewParam::getZoomRatio() const
{
  return m_viewProj.getZoom();
}

bool ZStackViewParam::onSamePlane(const ZStackViewParam &param) const
{
  bool result = false;
  if (m_sliceAxis == param.m_sliceAxis) {
    if (m_sliceAxis == neutube::A_AXIS) {
      result = zgeom::IsSameAffinePlane(
            m_center.toPoint(), m_v1, m_v2,
            param.m_center.toPoint(), param.m_v1, param.m_v2);
    } else {
      result = (m_z == param.m_z);
    }
  }

  return result;
}
