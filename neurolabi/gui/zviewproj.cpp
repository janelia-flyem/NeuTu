#include "zviewproj.h"

#include "tz_math.h"

ZViewProj::ZViewProj()
{
  init();
}

void ZViewProj::init()
{
  set(0, 0, 1.0);
}

bool ZViewProj::isValid() const
{
  return m_zoom > 0 && m_canvasRect.isValid() && m_widgetRect.isValid();
}

void ZViewProj::reset()
{
  set(0, 0, 1.0);
  setCanvasRect(QRect());
  setWidgetRect(QRect());
}

void ZViewProj::set(int x0, int y0, double zoom)
{
  setOffset(x0, y0);
  setZoom(zoom);
}

void ZViewProj::set(const QPoint &offset, double zoom)
{
  set(offset.x(), offset.y(), zoom);
}

void ZViewProj::deprecateViewPort() const
{
  m_viewPort.setWidth(0);
}

void ZViewProj::setCanvasRect(const QRect &canvasRect)
{
  m_canvasRect = canvasRect;
  deprecateViewPort();
}

void ZViewProj::setWidgetRect(const QRect &widgetRect)
{
  m_widgetRect = widgetRect;
  deprecateViewPort();
}

void ZViewProj::setOffset(int x0, int y0)
{
  m_x0 = x0;
  m_y0 = y0;
  deprecateViewPort();
}

void ZViewProj::setOffset(const QPoint &offset)
{
  setOffset(offset.x(), offset.y());
}

void ZViewProj::setZoom(double zoom)
{
  m_zoom = zoom;
  deprecateViewPort();
}

QRect ZViewProj::getCanvasRect() const
{
  return m_canvasRect;
}

QRect ZViewProj::getWidgetRect() const
{
  return m_widgetRect;
}

double ZViewProj::getZoom() const
{
  return m_zoom;
}

double ZViewProj::adjustProj(int vx, int cx, double px, double zoom) const
{
  int dx = cx - vx;
  double newPx = px + dx * zoom;

  return newPx;
}

double ZViewProj::adjustProjMin(int vx, int cx, double px, double zoom) const
{
  double newPx = px;

  if (vx < cx) {
    newPx = adjustProj(vx, cx, px, zoom);
  }

  return newPx;
}

double ZViewProj::adjustProjMax(int vx, int cx, double px, double zoom) const
{
  double newPx = px;

  if (vx > cx) {
    newPx = adjustProj(vx, cx, px, zoom);
  }

  return newPx;
}

QRectF ZViewProj::getProjRect() const
{
  if (m_viewPort.isEmpty()) {
    update();
  }

  return m_projRegion;
}

void ZViewProj::setViewPort(const QRect &rect)
{
  if (rect.isValid()) {
  setOffset(rect.topLeft());
  setZoom(std::min((double) (m_canvasRect.width()) / rect.width(),
                   (double) (m_canvasRect.height()) / rect.height()));
  }
}

QRect ZViewProj::getViewPort() const
{
  if (m_viewPort.isEmpty()) {
    update();
  }

  return m_viewPort;
}

bool ZViewProj::isSourceValid() const
{
  return m_canvasRect.isValid() && m_widgetRect.isValid();
}

void ZViewProj::maximizeViewPort()
{
  if (isSourceValid()) {
    double xZoom = (double) m_widgetRect.width() / m_canvasRect.width();
    double yZoom = (double) m_widgetRect.height() / m_canvasRect.height();
    setZoom(std::min(xZoom, yZoom));
    setOffset(m_canvasRect.left(), m_canvasRect.top());
  }
}

/*
void ZViewProj::setZoom(double zoom)
{
  setZoom(zoom);
  update();
}
*/


void ZViewProj::move(int dx, int dy)
{
  m_x0 += dx;
  m_y0 += dy;
}

void ZViewProj::setZoom(double zoom, EReference ref)
{
  if (zoom > 0) {
    if (ref == REF_CENTER) {
      update();
      QPoint viewCenter = getViewPort().center();
      QPointF projCenter = getProjRect().center();
      int cx = iround(projCenter.x() / zoom);
      int cy = iround(projCenter.y() / zoom);

      setOffset(viewCenter.x() - cx, viewCenter.y() - cy);
    }
  }

  setZoom(zoom);
}

double ZViewProj::getMaxZoomRatio() const
{
  if (isSourceValid()) {
    return std::max(m_widgetRect.width() / 16, m_widgetRect.height() / 16);
  }

  return 0.0;
}

double ZViewProj::getMinZoomRatio() const
{
  if (isSourceValid()) {
    return std::min(double(m_widgetRect.width()) * 0.5 / m_canvasRect.width(),
                    double(m_widgetRect.height()) * 0.5 / m_widgetRect.height());
  }

  return 0.0;
}

void ZViewProj::setZoomCapped(double zoom)
{
  double maxZoom = getMaxZoomRatio();
  if (zoom > maxZoom) {
    zoom = maxZoom;
  }

  double minZoom = getMinZoomRatio();
  if (zoom < minZoom) {
    zoom = minZoom;
  }

  setZoom(zoom);
}

void ZViewProj::increaseZoom()
{
  setZoomCapped(m_zoom * 1.1);
}

void ZViewProj::decreaseZoom()
{
  setZoomCapped(m_zoom / 1.1);
}

void ZViewProj::increaseZoom(int rx, int ry)
{
  double zoom = m_zoom * 1.1;

  double maxZoom = getMaxZoomRatio();

  if (zoom > maxZoom) {
    zoom = maxZoom;
  }
  setZoomWithFixedPoint(zoom, QPoint(rx, ry));
}

void ZViewProj::setZoomWithFixedPoint(
    double zoom, QPoint viewPoint, QPointF projPoint)
{
  if (zoom > 0) {
    int cx = iround(projPoint.x() / zoom);
    int cy = iround(projPoint.y() / zoom);

    setOffset(viewPoint.x() - cx, viewPoint.y() - cy);
  }

  setZoom(zoom);
}

void ZViewProj::setZoomWithFixedPoint(double zoom, QPoint viewPoint)
{
  setZoomWithFixedPoint(zoom, viewPoint, mapPoint(viewPoint));
}

QPointF ZViewProj::mapPoint(const QPoint &p)
{
  double x = (p.x() - m_x0) * m_zoom;
  double y = (p.y() - m_y0) * m_zoom;

  return QPointF(x, y);
}

QPoint ZViewProj::mapPointBack(const QPointF &p)
{
  if (m_zoom <= 0) {
    return QPoint(0, 0);
  }

  int x = iround(p.x() / m_zoom) + m_x0;
  int y = iround(p.y() / m_zoom) + m_y0;

  return QPoint(x, y);
}

void ZViewProj::update() const
{
  m_projRegion = QRectF();
  m_viewPort = QRect();

  if (isValid()) {
    //Compute the corresponding view port:
    //  lefttop -> (0,0), (wv, hv) -> (ww, hw)
    m_viewPort.setTopLeft(QPoint(m_x0, m_y0));
    m_viewPort.setSize(QSize(iround(m_widgetRect.width() / m_zoom),
                             iround(m_widgetRect.height() / m_zoom)));

    if (!m_viewPort.intersects(m_widgetRect)) {
      m_viewPort = QRect();
    } else {
      //Set initial proj region to the whole widget
      m_projRegion = m_widgetRect;

      //Adjust projection region to remove empty regions in view port
      if (m_viewPort.left() < m_canvasRect.left()) {
        m_projRegion.setLeft(
              adjustProj(m_viewPort.left(), m_canvasRect.left(),
                         m_projRegion.left(), m_zoom));
        m_viewPort.setLeft(m_canvasRect.left());
      }

      if (m_viewPort.right() > m_canvasRect.right()) {
        m_projRegion.setRight(
              adjustProj(m_viewPort.right(), m_canvasRect.right(),
                         m_projRegion.right(), m_zoom));
        m_viewPort.setRight(m_canvasRect.right());
      }

      if (m_viewPort.top() < m_canvasRect.top()) {
        m_projRegion.setTop(
              adjustProj(m_viewPort.top(), m_canvasRect.top(),
                         m_projRegion.top(), m_zoom));
        m_viewPort.setTop(m_canvasRect.top());
      }

      if (m_viewPort.bottom() > m_canvasRect.bottom()) {
        m_projRegion.setBottom(
              adjustProj(m_viewPort.bottom(), m_canvasRect.bottom(),
                         m_projRegion.bottom(), m_zoom));
        m_viewPort.setBottom(m_canvasRect.bottom());
      }
    }
  }
}
