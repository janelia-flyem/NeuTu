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

  enum EReference {
    REF_LEFTOP, REF_CENTER
  };

  void set(int x0, int y0, double zoom);
  void setCanvasRect(const QRect &canvasRect);
  void setWidgetRect(const QRect &widgetRect);
  void setOffset(int x0, int y0);
  void setZoom(double zoom);

  void update();

  QRectF getProjRegion() const;
  QRect getViewPort() const;

  bool isValid() const;
  bool isSourceValid() const;

  void maximizeViewPort();

  void updateZoom(double zoom);
  void updateZoom(double zoom, EReference ref);
  void updateZoomWithFixedPoint(
      double zoom, QPoint viewPoint, QPointF projPoint);

private:
  void init();
  double adjustProj(int vx, int cx, double px, double zoom);
  double adjustProjMin(int vx, int cx, double px, double zoom);
  double adjustProjMax(int vx, int cx, double px, double zoom);


private:
  int m_x0;
  int m_y0;
  double m_zoom; //the ratio from view to projection: p/v
  QRect m_canvasRect;
  QRect m_widgetRect;
  QRectF m_projRegion;
  QRect m_viewPort;
};

#endif // ZVIEWPROJ_H
