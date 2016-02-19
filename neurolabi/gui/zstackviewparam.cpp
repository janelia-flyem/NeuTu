#include "zstackviewparam.h"

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
}

void ZStackViewParam::setZ(int z)
{
  m_z = z;
}

void ZStackViewParam::setViewPort(const QRect &rect)
{
  m_viewPort = rect;
}

void ZStackViewParam::setViewPort(double x0, double y0, double x1, double y1)
{
  m_viewPort.setTopLeft(QPoint(x0, y0));
  m_viewPort.setBottomRight(QPoint(x1, y1));
}

void ZStackViewParam::setExploreAction(NeuTube::View::EExploreAction action)
{
  m_action = action;
}

bool ZStackViewParam::operator ==(const ZStackViewParam &param) const
{
  return m_z == param.m_z && m_coordSys == param.m_coordSys &&
      m_viewPort == param.m_viewPort;
}

bool ZStackViewParam::operator !=(const ZStackViewParam &param) const
{
  return m_z != param.m_z || m_coordSys != param.m_coordSys ||
      m_viewPort != param.m_viewPort;
}

bool ZStackViewParam::contains(const ZStackViewParam &param) const
{
  if (m_z == param.m_z) {
    return m_viewPort.contains(param.m_viewPort);
  }

  return false;
}

void ZStackViewParam::resize(int width, int height)
{
  QPoint oldCenter = m_viewPort.center();
  m_viewPort.setSize(QSize(width, height));
  m_viewPort.moveCenter(oldCenter);
}

int ZStackViewParam::getArea() const
{
  return m_viewPort.width() * m_viewPort.height();
}

void ZStackViewParam::setSliceAxis(NeuTube::EAxis sliceAxis)
{
  m_sliceAxis = sliceAxis;
}

NeuTube::EAxis ZStackViewParam::getSliceAxis() const
{
  return m_sliceAxis;
}
