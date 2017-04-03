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


void ZViewProj::set(int x0, int y0, double zoom)
{
  setOffset(x0, y0);
  setZoom(zoom);
}

void ZViewProj::setCanvasRect(const QRect &canvasRect)
{
  m_canvasRect = canvasRect;
}

void ZViewProj::setWidgetRect(const QRect &widgetRect)
{
  m_widgetRect = widgetRect;
}

void ZViewProj::setOffset(int x0, int y0)
{
  m_x0 = x0;
  m_y0 = y0;
}

void ZViewProj::setZoom(double zoom)
{
  m_zoom = zoom;
}

double ZViewProj::adjustProj(int vx, int cx, double px, double zoom)
{
  int dx = cx - vx;
  double newPx = px + dx * zoom;

  return newPx;
}

double ZViewProj::adjustProjMin(int vx, int cx, double px, double zoom)
{
  double newPx = px;

  if (vx < cx) {
    newPx = adjustProj(vx, cx, px, zoom);
  }

  return newPx;
}

double ZViewProj::adjustProjMax(int vx, int cx, double px, double zoom)
{
  double newPx = px;

  if (vx > cx) {
    newPx = adjustProj(vx, cx, px, zoom);
  }

  return newPx;
}

QRectF ZViewProj::getProjRegion() const
{
  return m_projRegion;
}

QRect ZViewProj::getViewPort() const
{
  return m_viewPort;
}

void ZViewProj::update()
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
