#ifndef ZVIEWPROJ_H
#define ZVIEWPROJ_H

#include <QRect>

/*!
 * \brief The class of handling view-projection computation
 */
class ZViewProj
{
public:
  ZViewProj();

  void set(int x0, int y0, double zoom);
  void setCanvasRect(const QRect &canvasRect);
  void setWidgetRect(const QRect &widgetRect);
  void setOffset(int x0, int y0);
  void setZoom(double zoom);

  QRectF getProjRegion() const;
  QRect getViewPort() const;

private:
  void init();

private:
  int m_x0;
  int m_y0;
  double m_zoom;
  QRect m_canvasRect;
  QRect m_widgetRect;
};

#endif // ZVIEWPROJ_H
