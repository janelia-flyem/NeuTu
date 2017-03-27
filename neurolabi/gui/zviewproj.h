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

  void reset();

  void set(int x0, int y0, double zoom);
  void set(const QPoint &offset, double zoom);
  void setCanvasRect(const QRect &canvasRect);
  void setWidgetRect(const QRect &widgetRect);
  void setOffset(int x0, int y0);
  void setZoom(double zoom);

  QRect getCanvasRect() const;
  QRect getWidgetRect() const;

  double getZoom() const;

  QRectF getProjRegion() const;
  QRect getViewPort() const;

  bool isValid() const;
  bool isSourceValid() const;

  void maximizeViewPort();

//  void updateZoom(double zoom);
  void setZoom(double zoom, EReference ref);
  void setZoomWithFixedPoint(
      double zoom, QPoint viewPoint, QPointF projPoint);
  void setZoomWithFixedPoint(double zoom, QPoint viewPoint);

  QPointF mapPoint(const QPoint &p);
  QPoint mapPointBack(const QPointF &p);

  double getMaxZoomRatio() const;
  double getMinZoomRatio() const;

  void increaseZoom();
  void decreaseZoom();

  /*!
   * \brief Zooming with reference point
   * \param rx
   * \param ry
   */
  void increaseZoom(int rx, int ry);
  void decreaseZoom(int rx, int ry);

  void move(int dx, int dy);

private:
  void init();
  double adjustProj(int vx, int cx, double px, double zoom) const;
  double adjustProjMin(int vx, int cx, double px, double zoom) const;
  double adjustProjMax(int vx, int cx, double px, double zoom) const;
  void deprecateViewPort() const;
  void update() const;
  void setZoomCapped(double zoom);

private:
  int m_x0;
  int m_y0;
  double m_zoom; //the ratio from view to projection: p/v
  QRect m_canvasRect;
  QRect m_widgetRect;

  mutable QRectF m_projRegion;
  mutable QRect m_viewPort;
};

#endif // ZVIEWPROJ_H
