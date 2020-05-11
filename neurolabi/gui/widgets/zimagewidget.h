/**@file zimagewidget.h
 * @brief Image widget
 * @author Ting Zhao
 */
#ifndef ZIMAGEWIDGET_H_
#define ZIMAGEWIDGET_H_

#include <memory>
#include <QImage>
#include <QWidget>
#include <QMenu>
#include <QVector>
#include <QKeyEvent>

#include "common/neutudefs.h"
#include "zviewproj.h"
#include "data3d/zsliceviewtransform.h"
#include "vis2d/zslicecanvas.h"

class QPaintEvent;
class ZPaintBundle;
class ZImage;
class ZPixmap;
class ZWidgetMessage;
class ZPainter;
class ZStackObject;
class ZIntCuboid;

/** A class of widget for image display.
 *  Sample usage:
 *    ...
 *    ZImageWidget widget = new ZImageWidget(parent_widget, NULL);
 *
 *   --               -----
 *  |view port  ==>  | project region
 *   --              |
 *                    -----
 */
class ZImageWidget : public QWidget {
  Q_OBJECT

public:
  ZImageWidget(QWidget *parent);
  virtual ~ZImageWidget() override;

  inline void setPaintBundle(ZPaintBundle *bd) { m_paintBundle = bd; }

  enum ECanvasRole {
    CANVAS_ROLE_IMAGE = 0, /**< For the main stack data */
    CANVAS_ROLE_TILE, /**< For tiles*/
    CANVAS_ROLE_MASK, /**< For pixel masks (e.g. segmentation layer) */
    CANVAS_ROLE_OBJECT, /**< For normal objects */
    CANVAS_ROLE_DYNAMIC_OBJECT, /**< For objects that need dynamic loading */
    CANVAS_ROLE_ACTIVE_DECORATION, /**< For objects moving with mouse */
    CANVAS_ROLE_WIDGET, /**< For object painted into widget directly */
    CANVAS_ROLE_COUNT, /**< The total number of roles */
  };

  std::shared_ptr<ZSliceCanvas> getCanvas(ECanvasRole role) const;

  void validateCanvas(ECanvasRole role);
  std::shared_ptr<ZSliceCanvas> getValidCanvas(ECanvasRole role);
  std::shared_ptr<ZSliceCanvas> getClearCanvas(ECanvasRole role);

  void setImage(ZImage *image);
//  void setObjectCanvas(ZPixmap *canvas);
//  ZSliceCanvas* getObjectCanvas() { return &m_objectCanvas; }
//  ZSliceCanvas* getTileCanvas() { return &m_tileCanvas; }
//  ZSliceCanvas* getDynamicObjectCanvas() { return &m_dynamicObjectCanvas; }
  void setMask(ZImage *mask, int channel);
//  void setTileCanvas(ZPixmap *canvas);
//  void setDynamicObjectCanvas(ZPixmap *canvas);
//  void setActiveDecorationCanvas(ZPixmap *canvas);
//  void removeCanvas(ZPixmap *canvas);
  void removeCanvas(ZImage *canvas);

  QSizeF getViewportSize() const;

  const ZSliceViewTransform& getSliceViewTransform() const;
  ZPoint transform(
      const ZPoint &pt, neutu::data3d::ESpace src, neutu::data3d::ESpace dst) const;
  void setSliceViewTransform(const ZSliceViewTransform &t);
  void setCutCenter(double x, double y, double z);
  void setCutCenter(const ZPoint &center);
  void setCutCenter(const ZIntPoint &center);
  void moveCutDepth(double dz);

  ZPoint getCutCenter() const;

  void recordTransform();

  bool freeMoving() const {
    return m_freeMoving;
  }

  /*!
   * \brief Reset the image widget by removing all canvases and view information.
   */
  void reset();

  enum EViewPortAdjust {
    VIEWPORT_NO_ADJUST, VIEWPORT_EXPAND, VIEWPORT_SHRINK
  };

  QPointF getAnchorPoint() const;
  ZPoint getAnchorPoint(neutu::data3d::ESpace space) const;

  /*!
   * \brief Get viewport in the model space
   */
  ZAffineRect getViewPort() const;

//  void setViewPort(const QRect &rect);
  void setProjRegion(const QRectF &rect);
  void setView(double zoomRatio, const QPoint &zoomOffset);
  void setView(const QRect &viewPort, const QRectF &projRegion);

  /*!
   * \brief Set view port offset
   *
   * Set the first corner of viewport to (\a x, \a y) in the world coordinate
   * system. The position will be adjusted if (\a x, \a y) is outside the canvas.
   */
//  void setViewPortOffset(int x, int y);
  void setViewPortCenterQuitely(int cx, int cy);

  /*
  const ZViewProj& getViewProj() const {
    return m_viewProj;
  }

  void setViewProj(const ZViewProj &viewProj) {
    m_viewProj = viewProj;
  }
  */

  /*
  void setViewProj(int x0, int y0, double zoom);
  void setViewProj(const QPoint &pt, double zoom);
  void resetViewProj(int x0, int y0, int w, int h);
  void resetViewProj(int x0, int y0, int w, int h, const QRect &viewPort);
  */

  /*!
   * \brief Move viewport.
   *
   * Move the current viewport so that the offset between its first corner and
   * the first corner of the canvas is (\a dx, \a dy).
   */
  void moveViewPort(int dx, int dy);
  void moveViewPort(const QPoint &src, const QPointF &dst);

  /*!
   * \brief Move viewport
   *
   * Move \a src in the model space to a widget point \a dst.
   */
  void moveViewPort(const ZPoint &src, const QPointF &dst);

  void setZoomRatio(double zoomRatio);
  //inline int zoomRatio() const { return m_zoomRatio; }
  void increaseZoomRatio();
  void decreaseZoomRatio();

  void increaseZoomRatio(int x, int y, bool usingRef = true);
  void decreaseZoomRatio(int x, int y, bool usingRef = true);

  void zoom(double zoomRatio);
  void zoom(double zoomRatio, EViewPortAdjust option);

  void zoomTo(const QPoint &center, int width);

  void setViewPort(const QRect &rect);

  void restoreFromBadView(const ZIntCuboid &worldRange);

  /*!
   * \brief Zoom an image at a fixed point
   *
   * Zoom an image by keeping the screen point \a ref relatively constant.
   */
  void zoom(double zoomRatio, const QPointF &ref);
//  void zoom(double zoomRatio, const QPointF &ref, EViewPortAdjust option);

//  void zoomWithWidthAligned(int x0, int x1, int cy);
//  void zoomWithWidthAligned(int x0, int x1, double pw, int cy);
//  void zoomWithHeightAligned(int y0, int y1, double ph, int cx);

  void setModelRange(const ZIntCuboid &range);
//  void setCanvasRegion(int x0, int y0, int w, int h);

  //void setData(const uchar *data, int width, int height, QImage::Format format);
  QSize minimumSizeHint() const override;
  QSize sizeHint() const override;
//  bool isColorTableRequired();
  void addColorTable();

//  QSize canvasSize() const;
  QSize screenSize() const;
//  inline QSizeF projectSize() const { return m_projRegion.size(); }
//  inline const QRectF& projectRegion() const { return m_projRegion; }
//  inline const QRect& viewPort() const { return m_viewPort; }
//  QSizeF projectSize() const;
//  QRectF projectRegion() const;
//  QRect viewPort() const;
//  QRect canvasRegion() const;


  /*!
   * \brief Map the widget coordinates to world coordinates
   */
//  QPointF worldCoordinate(QPoint widgetCoord) const;

//  QPointF canvasCoordinate(QPoint widgetCoord) const;


  void paintEvent(QPaintEvent *event) override;

  bool popLeftMenu(const QPoint &pos);
  bool popRightMenu(const QPoint &pos);

  bool showContextMenu(QMenu *menu, const QPoint &pos);

  QMenu* leftMenu();
  QMenu* rightMenu();

  ///{
  /// The actual ration and offset are calculated from the current view port
  /// and project region.
  ///
  //Formula: x0' = x0 * ratio + offset
  /*!
   * \brief Get the actual zoom ratio along X (horizontal)
   */
//  double getAcutalZoomRatioX() const;
  /*!
   * \brief Get the actual offset along X (horizontal)
   */
  double getActualOffsetX() const;
  /*!
   * \brief Get the actual zoom ratio along Y (vertical)
   */
//  double getAcutalZoomRatioY() const;
  /*!
   * \brief Get the actual offset along Y (vertical)
   */
  double getActualOffsetY() const;
  ///}

  inline void setViewHintVisible(bool visible) {
    m_isViewHintVisible = visible;
  }

  inline void blockPaint(bool state) {
    m_paintBlocked = state;
  }

  bool isPaintBlocked() const {
    return m_paintBlocked;
  }

  void setSliceAxis(neutu::EAxis axis);

  neutu::EAxis getSliceAxis() const;

  void setHoverFocus(bool on) {
    m_hoverFocus = on;
  }

  void setSmoothDisplay(bool on) {
    m_smoothDisplay = on;
  }

  void hideZoomHint();

  void showCrossHair(bool on);
  void updateCrossHair(int x, int y);

  void maximizeViewPort(const ZIntCuboid &worldRange);

  void enableOffsetAdjustment(bool on);
  bool paintWidgetCanvas(ZImage *canvas);
  ZImage* makeWidgetCanvas() const;
  void updateWidgetCanvas(ZPixmap *canvas);

  //To be called by parent widget
  void adjustTransformWithResize();
  void adjustMinScale();

  int getCutDepth() const;
  int getMinCutDepth() const;
  int getMaxCutDepth() const;

//  void resetTransform();

public:
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

protected:
  void keyPressEvent(QKeyEvent *event) override;
  bool event(QEvent *event) override;
  void showEvent(QShowEvent *event) override;

public slots:
  void updateView();

signals:
  void mouseReleased(QMouseEvent*);
  void mouseMoved(QMouseEvent*);
  void mousePressed(QMouseEvent*);
  void mouseDoubleClicked(QMouseEvent*);
  void mouseWheelRolled(QWheelEvent *event);
  void messageGenerated(const ZWidgetMessage&);
  void transformChanged();
  void transformControlSyncNeeded();

protected:
//  int getMaxZoomRatio() const;

private:
  void init();

  void setValidViewPortBackup(const QRect &viewPort);
  void setValidViewPort(const QRect &viewPort);
  /*!
   * \brief Align view port by aligning a point
   *
   * Align the view port to map the world coordinates (\a vx, \a vy) to the
   * screen coordinates (\a px, \a py).
   */
  QRect alignViewPort(
      const QRect &viewPort, double vx, double vy, double px, double py,
      double ratio) const;
  QRect alignViewPort(
      const QRect &viewPort, int vx, int vy, int px, int py) const;

  void alignProjRegion(double ratio);


  QRect adjustViewPort(const QRect &viewPort, EViewPortAdjust option);
  void adjustViewPort(EViewPortAdjust option);

  /*!
   * \brief Maximize the projection region while ensuring that at least one of
   *        the deminesions of the view port is fully covered.
   */
  void adjustProjRegion();
  void adjustProjRegion(const QRect &viewPort);
  QSize getMaskSize() const;
  void paintObject();
  void paintDynamicObject();
  bool paintObject(QPainter *painter,
      const QList<std::shared_ptr<ZStackObject>> &objList);
  bool paintObject(QPainter *painter,
      const QList<ZStackObject*> &objList);
  template<typename ZStackObjectPtr>
  bool paintObjectTmpl(QPainter *painter, const QList<ZStackObjectPtr> &objList);
  void paintZoomHint();
  void paintCrossHair();

  bool isBadView() const;

  bool isModelWithinWidget() const;

  void notifyTransformChanged();

private:
  ZImage *m_image = nullptr;
  QVector<ZImage*> m_mask;

  QVector<std::shared_ptr<ZSliceCanvas>> m_canvasList;
  /*
  std::shared_ptr<ZSliceCanvas> m_objectCanvas;
  std::shared_ptr<ZSliceCanvas> m_tileCanvas;
  std::shared_ptr<ZSliceCanvas> m_dynamicObjectCanvas;
  std::shared_ptr<ZSliceCanvas> m_activeDecorationCanvas;
  std::shared_ptr<ZSliceCanvas> m_widgetCanvas;
  */

//  QRect m_viewPort; /* viewport, in world coordinates */
//  QRectF m_projRegion; /* projection region */
  //int m_zoomRatio;
//  bool m_isowner;
  QMenu *m_leftButtonMenu = nullptr;
  QMenu *m_rightButtonMenu = nullptr;
  ZPaintBundle *m_paintBundle = nullptr;
  bool m_isViewHintVisible = true;
  bool m_paintBlocked = false;
//  QRect m_canvasRegion; //Whole canvas region

  Qt::MouseButtons m_pressedButtons = Qt::NoButton;

//  ZViewProj m_viewProj;
  ZIntCuboid m_modelRange;
  ZSliceViewTransform m_sliceViewTransform;
  ZSliceViewTransform m_prevSliceViewTransform;
  double m_viewAnchorX = 0.5; //anchor point (defined as window size ratio)
  double m_viewAnchorY = 0.5;

  neutu::EAxis m_sliceAxis = neutu::EAxis::Z;
//  QSize m_canvasSize;

  bool m_freeMoving = true;
  bool m_hoverFocus = false;
  bool m_smoothDisplay = false;
  bool m_showingCrossHair= false;
  bool m_showingZoomHint = true;
  bool m_isReady = false;
  bool m_offsetAdjustment = false;
  QPoint m_hairCenter;
};

#endif
