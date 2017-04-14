#include "zviewproj.h"

#include <cmath>
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

void ZViewProj::setX0(int x)
{
  m_x0 = x;
}

void ZViewProj::setY0(int y)
{
  m_y0 = y;
}

QRect ZViewProj::getCanvasRect() const
{
  return m_canvasRect;
}

QRect ZViewProj::getWidgetRect() const
{
  return m_widgetRect;
}

QPoint ZViewProj::getOffset() const
{
  return QPoint(m_x0, m_y0);
}

int ZViewProj::getX0() const
{
  return m_x0;
}

int ZViewProj::getY0() const
{
  return m_y0;
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
//    setOffset(rect.topLeft());
    setZoom(std::min((double) (m_widgetRect.width()) / rect.width(),
                     (double) (m_widgetRect.height()) / rect.height()));
    setViewCenter(rect.center());
  }
}

void ZViewProj::zoomTo(int x, int y, int width)
{
  int radius = width / 2;

  double zoom = std::min((double) (m_widgetRect.width()) / width,
                         (double) (m_widgetRect.height()) / width);

  if (zoom > getZoom()) {
    setOffset(x - radius, y - radius);
    setZoom(zoom);
  } else {
    setViewCenter(x, y);
  }


//  QRect rect(x - radius, y - radius, width, width);
//  setViewPort(rect);
}

void ZViewProj::zoomTo(const QPoint &pt, int width)
{
  zoomTo(pt.x(), pt.y(), width);
}

QRect ZViewProj::getViewPort() const
{
  if (m_viewPort.isEmpty()) {
    update();
  }

  return m_viewPort;
}

QPoint ZViewProj::getWidgetCenter() const
{
  return m_widgetRect.center();
}

bool ZViewProj::isSourceValid() const
{
  return m_canvasRect.isValid() && m_widgetRect.isValid();
}

void ZViewProj::maximizeViewPort()
{
  if (isSourceValid()) {
    double xZoom = double(m_widgetRect.width()) / m_canvasRect.width();
    double yZoom = double(m_widgetRect.height()) / m_canvasRect.height();

    setZoom(std::min(xZoom, yZoom));

    if (xZoom == yZoom) {
      m_x0 = m_canvasRect.left();
      m_y0 = m_canvasRect.top();
    } else {
      QPoint widgetCenter = m_widgetRect.center();
      QPoint canvasCenter = m_canvasRect.center();

      if (xZoom > yZoom) {
        move(canvasCenter.x(), m_canvasRect.top(),
             widgetCenter.x(), m_widgetRect.top());
      } else {
        move(m_canvasRect.left(), canvasCenter.y(),
             m_widgetRect.left(), widgetCenter.y());
      }
    }
  }
}

/*
void ZViewProj::setZoom(double zoom)
{
  setZoom(zoom);
  update();
}
*/

void ZViewProj::recoverViewPort()
{
  if (getViewPort().contains(m_canvasRect)) {
    maximizeViewPort();
  }
}

void ZViewProj::move(int dx, int dy)
{
  m_x0 += dx;
  m_y0 += dy;
}

double ZViewProj::getValidZoom(double zoom) const
{
  double maxZoom = getMaxZoomRatio();
  if (zoom > maxZoom) {
    zoom = maxZoom;
  } else {
    double minZoom = getMinZoomRatio();
    if (zoom < minZoom) {
      zoom = minZoom;
    }
  }

  return zoom;
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
                    double(m_widgetRect.height()) * 0.5 / m_canvasRect.height());
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
  double zoom = getValidZoom(m_zoom * 1.1);

  setZoomWithFixedPoint(zoom, QPoint(rx, ry));
}

void ZViewProj::decreaseZoom(int rx, int ry)
{
  double zoom = getValidZoom(m_zoom / 1.1);

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

void ZViewProj::move(int srcX, int srcY, double dstX, double dstY)
{
  if (m_zoom > 0.0) {
    int x0 = srcX - iround(dstX / m_zoom);
    int y0 = srcY - iround(dstY / m_zoom);

    setOffset(x0, y0);
  }
}

void ZViewProj::move(const QPoint &src, const QPointF &dst)
{
  move(src.x(), src.y(), dst.x(), dst.y());
}

void ZViewProj::move(const QPoint &src, const QPoint &dst)
{
  move(src.x(), src.y(), dst.x(), dst.y());
}

void ZViewProj::setViewCenter(int x, int y)
{
  move(QPoint(x, y), getWidgetCenter());
}

void ZViewProj::setViewCenter(const QPoint &pt)
{
  setViewCenter(pt.x(), pt.y());
}

QPointF ZViewProj::mapPoint(const QPoint &p)
{
  double x = (p.x() - m_x0) * m_zoom;
  double y = (p.y() - m_y0) * m_zoom;

  return QPointF(x, y);
}

QPointF ZViewProj::mapPoint(const QPointF &p)
{
  double x = (p.x() - m_x0) * m_zoom;
  double y = (p.y() - m_y0) * m_zoom;

  return QPointF(x, y);
}

void ZViewProj::mapPointBack(double *x, double *y)
{
  if (m_zoom > 0.0) {
    *x = *x / m_zoom + m_x0;
    *y = *y / m_zoom + m_y0;
  } else {
    *x = 0.0;
    *y = 0.0;
  }
}

QPointF ZViewProj::mapPointBackF(const QPointF &p)
{
  if (m_zoom <= 0) {
    return QPointF(0, 0);
  }

  double x = p.x() / m_zoom + m_x0;
  double y = p.y() / m_zoom + m_y0;

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

    m_viewPort.setSize(QSize(std::ceil(m_widgetRect.width() / m_zoom),
                             std::ceil(m_widgetRect.height() / m_zoom)));

    if (!m_viewPort.intersects(m_canvasRect)) {
      m_viewPort = QRect();
    } else {
      //Set initial proj region to the whole widget
//      m_projRegion = m_widgetRect;
      m_projRegion = QRectF(
            0, 0, m_viewPort.width() * m_zoom, m_viewPort.height() * m_zoom);

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
