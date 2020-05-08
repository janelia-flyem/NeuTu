#ifndef ZVIEWPROJ_H
#define ZVIEWPROJ_H

#include <QRect>
#include <QTransform>

class ZJsonObject;

/*!
 * \brief The class of handling view-projection computation
 */
class ZViewProj
{
public:
  ZViewProj();

  enum class EReference {
    LEFTOP, CENTER
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

  void alignOffset(int downresRatio);

  QRect getCanvasRect() const;
  QRect getWidgetRect() const;
  QPoint getOffset() const;
  int getX0() const;
  int getY0() const;
  double getZoom() const;

  QRectF getProjRect() const;
  QRect getViewPort() const;
  QPoint getViewPortCenter() const;

  QPoint getWidgetCenter() const;

  bool isValid() const;
  bool isSourceValid() const;

  void maximizeViewPort();

//  void updateZoom(double zoom);
  void setZoom(double zoom, EReference ref);
  void setZoomWithFixedPoint(
      double zoom, const QPoint &viewPoint, const QPointF &projPoint);
  void setZoomWithFixedPoint(double zoom, const QPoint &viewPoint);
//  void setZoomWithFixedPoint(double zoom, const QPointF &viewPoint);

  /*!
   * \brief move a viewport position to a widget postion.
   *
   * After moving, the viewport position (\a srcX, \a srcY) should be mapped to
   * the widget position (\a dstX, \a dstY).
   */
  void move(int srcX, int srcY, double dstX, double dstY);
  void move(const QPoint &src, const QPointF &dst);
  void move(const QPoint &src, const QPoint &dst);
  void setViewCenter(int x, int y);
  void setViewCenter(const QPoint &pt);
//  void setViewCenter(const QPointF &pt);

  /*!
   * \brief Map a point from viewport to projection area
   */
  QPointF mapPoint(const QPoint &p) const;
  QPointF mapPoint(const QPointF &p) const;

  /*!
   * \brief Map a point from projection to viewport
   */
  QPoint mapPointBack(const QPointF &p) const;
  QPointF mapPointBackF(const QPointF &p) const;
  void mapPointBack(double *x, double *y) const;

  double getMaxZoomRatio() const;
  double getMinZoomRatio() const;

  void increaseZoom();
  void decreaseZoom();

  void setViewPort(const QRect &rect);
//  void setNullViewPort();
  void openViewPort();
  void closeViewPort();
//  void setViewPortWithZoomFixed(const QRect &rect);

  /*!
   * \brief Cache a viewport for future setting
   */
  void prepareViewPort(const QRect &rect);

  /*!
   * \brief Store the current viewport into the buffer
   */
//  void backupViewPort();

  void zoomTo(int x, int y, int width);
  void zoomTo(const QPoint &pt, int width);

  /*!
   * \brief Zooming with reference point
   *
   * \param rx X coordinate of reference point.
   * \param ry Y coordinate of reference point.
   */
  void increaseZoom(int rx, int ry);
  void decreaseZoom(int rx, int ry);

  void move(int dx, int dy);

  /*!
   * \brief Reset the viewport if it contains the whole canvas.
   */
  void recoverViewPort();

  void print() const;
  ZJsonObject toJsonObject() const;

  bool operator ==(const ZViewProj &viewProj) const;
  bool operator !=(const ZViewProj &viewProj) const;

  /*!
   * \brief View space to widget transform
   */
  QTransform getViewWidgetTransform() const;

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
  double m_zoomBackup = 0; //for viewport closing and opening
  QRect m_canvasRect;
  QRect m_widgetRect;

  QRect m_viewPortBuffer; //The buffer is used to force viewport setting
                          //It will be removed if m_viewPort becomes a canonical
                          //member in the future.

  mutable QRectF m_projRegion;
  mutable QRect m_viewPort;
};

#endif // ZVIEWPROJ_H
