#include "zstackviewparam.h"

#include <cmath>

#include "geometry/zgeometry.h"
#include "tz_math.h"

ZStackViewParam::ZStackViewParam()
{
  init(NeuTube::COORD_RAW_STACK);
}

ZStackViewParam::ZStackViewParam(NeuTube::ECoordinateSystem coordSys)
{
  init(coordSys);
}

void ZStackViewParam::init(NeuTube::ECoordinateSystem coordSys)
{
  m_z = 0;
  m_coordSys = coordSys;
  m_action = NeuTube::View::EXPLORE_UNKNOWN;
  m_fixingZ = false;
  m_sliceAxis = NeuTube::Z_AXIS;
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

void ZStackViewParam::setViewProj(const ZViewProj &vp)
{
  m_viewProj = vp;
}


void ZStackViewParam::setViewPort(const QRect &rect)
{
  m_viewProj.setViewPort(rect);
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

void ZStackViewParam::setExploreAction(NeuTube::View::EExploreAction action)
{
  m_action = action;
}

bool ZStackViewParam::operator ==(const ZStackViewParam &param) const
{
  return m_z == param.m_z && m_coordSys == param.m_coordSys &&
      getViewPort() == param.getViewPort();
}

bool ZStackViewParam::operator !=(const ZStackViewParam &param) const
{
  return m_z != param.m_z || m_coordSys != param.m_coordSys ||
      getViewPort() != param.getViewPort();
}

bool ZStackViewParam::contains(const ZStackViewParam &param) const
{
  if (m_z == param.m_z) {
    return getViewPort().contains(param.getViewPort());
  }

  return false;
}

bool ZStackViewParam::containsViewport(const ZStackViewParam &param) const
{
  return getViewPort().contains(param.getViewPort());
}

bool ZStackViewParam::contains(int x, int y, int z)
{
  ZGeometry::shiftSliceAxis(x, y, z, getSliceAxis());

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

int ZStackViewParam::getArea() const
{
  return getViewPort().width() * getViewPort().height();
}

void ZStackViewParam::setSliceAxis(NeuTube::EAxis sliceAxis)
{
  m_sliceAxis = sliceAxis;
}

NeuTube::EAxis ZStackViewParam::getSliceAxis() const
{
  return m_sliceAxis;
}

int ZStackViewParam::getZoomLevel(int maxLevel) const
{
  int zoom = std::round(std::log(1.0 / getZoomRatio()) / std::log(2.0) ) -1;

  if (zoom < 0) {
    zoom = 0;
  }

  int scale = pow(2, zoom);
  if (getViewPort().width() * getViewPort().height() /
      scale / scale > 1024 * 1024) {
    zoom += 1;
  }

  if (zoom > maxLevel) {
    zoom = maxLevel;
  }

  return zoom;
}

double ZStackViewParam::getZoomRatio() const
{
  return m_viewProj.getZoom();
}
