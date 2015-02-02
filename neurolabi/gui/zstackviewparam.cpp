#include "zstackviewparam.h"

ZStackViewParam::ZStackViewParam() :
  m_z(0), m_coordSys(NeuTube::COORD_RAW_STACK)
{
}

ZStackViewParam::ZStackViewParam(NeuTube::ECoordinateSystem coordSys) :
  m_z(0), m_coordSys(coordSys)
{
}

void ZStackViewParam::setZ(int z)
{
  m_z = z;
}

void ZStackViewParam::setViewPort(const QRect &rect)
{
  m_viewPort = rect;
}

void ZStackViewParam::setViewPort(int x0, int y0, int x1, int y1)
{
  m_viewPort.setTopLeft(QPoint(x0, y0));
  m_viewPort.setBottomRight(QPoint(x1, y1));
}
