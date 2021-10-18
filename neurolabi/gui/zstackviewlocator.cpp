#include "zstackviewlocator.h"

#include <algorithm>

#include "common/math.h"
#include "common/utilities.h"

ZStackViewLocator::ZStackViewLocator() : m_sceneRatio(0.1), m_minZoomRatio(2)
{
}

void ZStackViewLocator::setCanvasSize(int w, int h)
{
  m_canvasSize.setWidth(w);
  m_canvasSize.setHeight(h);
}

void ZStackViewLocator::setSceneRatio(double ratio)
{
  m_sceneRatio = neulib::ClipValue(ratio, 0.01, 1.0);
}

int ZStackViewLocator::getZoomRatio(int viewPortWidth, int viewPortHeight) const
{
  int zoomRatio = 1;

  if (viewPortWidth > 0 && viewPortHeight > 0) {
    zoomRatio = std::min(m_canvasSize.width() / viewPortWidth,
             m_canvasSize.height() / viewPortHeight);
    if (zoomRatio < m_minZoomRatio) {
      zoomRatio = m_minZoomRatio;
    }
  }

  return zoomRatio;
}

QRect ZStackViewLocator::getRectViewPort(double cx, double cy, double width) const
{
  QRect port;

  int left = neutu::iround(cx - width / 2);
  if (left < 0) {
    left = 0;
  }
  int top = neutu::iround(cy - width / 2);
  if (top < 0) {
    top = 0;
  }

  port.setTop(top);
  port.setLeft(left);

  int right = neutu::iround(cx + width / 2);
  if (right >= m_canvasSize.width()) {
    right = m_canvasSize.width() - 1;
  }
  int bottom = neutu::iround(cy + width / 2);
  if (bottom >= m_canvasSize.height()) {
    bottom = m_canvasSize.height() - 1;
  }

  int zoomRatio = getZoomRatio(right - left, bottom - top);

  port.setWidth(m_canvasSize.width() / zoomRatio);
  port.setHeight(m_canvasSize.height() / zoomRatio);

  return port;
}

QRect ZStackViewLocator::getLandmarkViewPort(double cx, double cy, double radius) const
{
  return getRectViewPort(cx, cy, (radius + radius) / m_sceneRatio);
  /*
  QRect port;
  double width = ;

  int left = iround(cx - width / 2);
  if (left < 0) {
    left = 0;
  }
  int top = iround(cy - width / 2);
  if (top < 0) {
    top = 0;
  }

  port.setTop(top);
  port.setLeft(left);

  int right = iround(cx + width / 2);
  if (right >= m_canvasSize.width()) {
    right = m_canvasSize.width() - 1;
  }
  int bottom = iround(cy + width / 2);
  if (bottom >= m_canvasSize.height()) {
    bottom = m_canvasSize.height() - 1;
  }

  int zoomRatio = getZoomRatio(right - left, bottom - top);

  port.setWidth(m_canvasSize.width() / zoomRatio);
  port.setHeight(m_canvasSize.height() / zoomRatio);

  return port;
  */
}
