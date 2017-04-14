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
  void setOffset(const QPoint &offset);
  void setZoom(double zoom);
  void setX0(int x);
  void setY0(int y);

  QRect getCanvasRect() const;
  QRect getWidgetRect() const;
  QPoint getOffset() const;
  int getX0() const;
  int getY0() const;
  double getZoom() const;

  QRectF getProjRect() const;
  QRect getViewPort() const;

  QPoint getWidgetCenter() const;

  bool isValid() const;
  bool isSourceValid() const;

  void maximizeViewPort();

//  void updateZoom(double zoom);
  void setZoom(double zoom, EReference ref);
  void setZoomWithFixedPoint(
      double zoom, QPoint viewPoint, QPointF projPoint);
  void setZoomWithFixedPoint(double zoom, QPoint viewPoint);

  void move(int srcX, int srcY, double dstX, double dstY);
  void move(const QPoint &src, const QPointF &dst);
  void move(const QPoint &src, const QPoint &dst);
  void setViewCenter(int x, int y);
  void setViewCenter(const QPoint &pt);
//  void setViewCenter(const QPointF &pt);

  QPointF mapPoint(const QPoint &p);
  QPointF mapPoint(const QPointF &p);
  QPoint mapPointBack(const QPointF &p);
  QPointF mapPointBackF(const QPointF &p);
  void mapPointBack(double *x, double *y);

  double getMaxZoomRatio() const;
  double getMinZoomRatio() const;

  void increaseZoom();
  void decreaseZoom();

  void setViewPort(const QRect &rect);

  void zoomTo(int x, int y, int width);
  void zoomTo(const QPoint &pt, int width);



  /*!
   * \brief Zooming with reference point
   * \param rx
   * \param ry
   */
  void increaseZoom(int rx, int ry);
  void decreaseZoom(int rx, int ry);

  void move(int dx, int dy);

  void recoverViewPort();

private:
  void init();
  double adjustProj(int vx, int cx, double px, double zoom) const;
  double adjustProjMin(int vx, int cx, double px, double zoom) const;
  double adjustProjMax(int vx, int cx, double px, double zoom) const;
  void deprecateViewPort() const;
  void update() const;
  void setZoomCapped(double zoom);
  double getValidZoom(double zoom) const;

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
