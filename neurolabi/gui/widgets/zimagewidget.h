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

//  inline void setPaintBundle(ZPaintBundle *bd) { m_paintBundle = bd; }

#if 0
  enum ECanvasRole {
    CANVAS_ROLE_IMAGE = 0, /**< For the main stack data */
    CANVAS_ROLE_TILE, /**< For tiles*/
    CANVAS_ROLE_MASK, /**< For pixel masks (e.g. segmentation layer) */
    CANVAS_ROLE_OBJECT, /**< For normal objects */
    CANVAS_ROLE_DYNAMIC_OBJECT, /**< For objects that need dynamic loading */
    CANVAS_ROLE_ACTIVE_DECORATION, /**< For objects moving with mouse */
    CANVAS_ROLE_WIDGET, /**< For object painted into widget (for high definition) */
    CANVAS_ROLE_COUNT, /**< The total number of roles */
  };
#endif

  std::shared_ptr<ZSliceCanvas> getCanvas(
      neutu::data3d::ETarget target, bool initing);

  std::shared_ptr<ZSliceCanvas> validateCanvas(neutu::data3d::ETarget target);
  std::shared_ptr<ZSliceCanvas> getValidCanvas(neutu::data3d::ETarget target);
  std::shared_ptr<ZSliceCanvas> getClearCanvas(neutu::data3d::ETarget target);

  bool hasCanvas(ZSliceCanvas *canvas, neutu::data3d::ETarget target) const;

  ZSliceCanvas* makeClearCanvas();

  void setCanvasVisible(neutu::data3d::ETarget target, bool visible);

  void setImage(ZImage *image);
  void setMask(ZImage *mask, int channel);

//  void removeCanvas(ZImage *canvas);

  void setInitialScale(double s);

  QSizeF getViewportSize() const;

  double getCutDepth() const;
  int getMinCutDepth() const;
  int getMaxCutDepth() const;

  const ZSliceViewTransform& getSliceViewTransform() const;
  ZPoint transform(
      const ZPoint &pt, neutu::data3d::ESpace src, neutu::data3d::ESpace dst) const;
  void setSliceViewTransform(const ZSliceViewTransform &t);
  void setCutPlane(neutu::EAxis axis);
  void setRightHanded(bool r);
  void setCutPlane(const ZPoint &v1, const ZPoint &v2);
  void setCutPlane(const ZAffinePlane &plane);
  ZPlane getCutOrientation() const;

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
//  void setProjRegion(const QRectF &rect);
//  void setView(double zoomRatio, const QPoint &zoomOffset);
//  void setView(const QRect &viewPort, const QRectF &projRegion);

  /*!
   * \brief Set view port offset
   *
   * Set the first corner of viewport to (\a x, \a y) in the world coordinate
   * system. The position will be adjusted if (\a x, \a y) is outside the canvas.
   */
//  void setViewPortOffset(int x, int y);
//  void setViewPortCenterQuitely(int cx, int cy);


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
  void moveViewPortToCenter(const ZPoint &src);

  void setZoomRatio(double zoomRatio);
  //inline int zoomRatio() const { return m_zoomRatio; }
  void increaseZoomRatio();
  void decreaseZoomRatio();

  void increaseZoomRatio(int x, int y, bool usingRef = true);
  void decreaseZoomRatio(int x, int y, bool usingRef = true);

  void zoom(double zoomRatio);
  void zoom(double zoomRatio, EViewPortAdjust option);

  void zoomTo(const QPoint &center, int w, int h);
  void zoomTo(const ZPoint &pt, double w, double h, neutu::data3d::ESpace space);

  void rotate(double au, double av, double rad);
  void rotate(double da, double db);

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
  ZIntCuboid getModelRange() const;
//  void setCanvasRegion(int x0, int y0, int w, int h);

  //void setData(const uchar *data, int width, int height, QImage::Format format);
  QSize minimumSizeHint() const override;
  QSize sizeHint() const override;
//  bool isColorTableRequired();
  void addColorTable();

//  QSize canvasSize() const;
  QSize screenSize() const;


  ZPoint getCurrentMousePosition(neutu::data3d::ESpace space) const;
  bool containsCurrentMousePostion() const;

  void paintEvent(QPaintEvent *event) override;

  bool popLeftMenu(const QPoint &pos);
  bool popRightMenu(const QPoint &pos);

  bool showContextMenu(QMenu *menu, const QPoint &pos);

  QMenu* leftMenu();
  QMenu* rightMenu();

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
//  bool paintWidgetCanvas(ZImage *canvas);
  ZImage* makeWidgetCanvas() const;
  void updateWidgetCanvas(ZPixmap *canvas);

  void updateSliceCanvas(neutu::data3d::ETarget target, ZSliceCanvas *canvas);

  //To be called by parent widget
  void adjustTransformWithResize();
  void adjustMinScale();

  void resetView(double defaultScale = 0.0);
  void setReady(bool ready);
  bool isReady() const;

//  void paintWidgetObject();

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
  void transformSyncNeeded();
  void transformControlSyncNeeded();
  void sliceAxisChanged();

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
//  QSize getMaskSize() const;
//  void paintObject();
  bool paintObject(QPainter *painter,
      const QList<std::shared_ptr<ZStackObject>> &objList);
  bool paintObject(QPainter *painter,
      const QList<ZStackObject*> &objList);
  template<typename ZStackObjectPtr>
  bool paintObjectTmpl(QPainter *painter, const QList<ZStackObjectPtr> &objList);
  void paintZoomHint();
  void paintCrossHair();
  void paintAxis();

  bool isBadView() const;

  bool isModelWithinWidget() const;

  void blockTransformSyncSignal(bool blocking);
  void notifyTransformChanged();

private:
  ZImage *m_image = nullptr;
  QVector<ZImage*> m_mask;

  QVector<std::shared_ptr<ZSliceCanvas>> m_canvasList;

  QMenu *m_leftButtonMenu = nullptr;
  QMenu *m_rightButtonMenu = nullptr;
//  ZPaintBundle *m_paintBundle = nullptr;
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
  ZPlane m_defaultArbPlane;
  double m_initScale = 0.0;

  neutu::EAxis m_sliceAxis = neutu::EAxis::Z;
//  QSize m_canvasSize;

  bool m_freeMoving = true;
  bool m_hoverFocus = false;
  bool m_smoothDisplay = false;
  bool m_showingCrossHair= false;
  bool m_showingZoomHint = true;
  bool m_isReady = false;
  bool m_offsetAdjustment = false;
  bool m_signalingTransformSync = true;
  QPoint m_hairCenter;
};

#endif
