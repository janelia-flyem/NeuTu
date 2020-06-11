#include "zimagewidget.h"

#include <cstring>
#include <cmath>

#include <QGraphicsBlurEffect>
#include <QMouseEvent>
#include <QElapsedTimer>

#include "common/math.h"
#include "tz_rastergeom.h"
#include "misc/miscutility.h"
#include "geometry/2d/rectangle.h"

#include "logging/zqslog.h"
#include "logging/zlog.h"
#include "qt/gui/loghelper.h"

#include "zpainter.h"
#include "zpaintbundle.h"
#include "neutubeconfig.h"
#include "zimage.h"
#include "zpixmap.h"
#include "zstackobjectpainter.h"
#include "vis2d/zslicepainter.h"
#include "data3d/displayconfig.h"


ZImageWidget::ZImageWidget(QWidget *parent) : QWidget(parent)
{
  init();
}

ZImageWidget::~ZImageWidget()
{
//  delete m_widgetCanvas;
}

void ZImageWidget::init()
{
//  qDebug() << "ZImageWidget initialization:" << this;

//  m_isViewHintVisible = true;
//  m_freeMoving = true;
//  m_hoverFocus = false;
//  m_smoothDisplay = false;
//  m_isReady = false;

#if 0
  if (image != NULL) {
    m_viewPort.setRect(0, 0, image->width(), image->height());
  }

  m_projRegion.setRect(0, 0, 0, 0);
#endif
  //m_zoomRatio = 1;

  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  setAttribute(Qt::WA_OpaquePaintEvent);
  //setAttribute(Qt::WA_NoSystemBackground);
//  setImage(NULL);
  setCursor(Qt::CrossCursor);
  setMouseTracking(true);
  m_leftButtonMenu = new QMenu(this);
  m_rightButtonMenu = new QMenu(this);

  m_canvasList.resize(CANVAS_ROLE_COUNT);

  m_sliceViewTransform.setMinScale(0.4);
//  m_paintBundle = NULL;
//  m_tileCanvas = NULL;
//  m_objectCanvas = NULL;
//  m_dynamicObjectCanvas = NULL;
//  m_activeDecorationCanvas = NULL;
//  m_widgetCanvas = NULL;

//  m_sliceAxis = neutube::EAxis::Z;
}

std::shared_ptr<ZSliceCanvas> ZImageWidget::getCanvas(ECanvasRole role) const
{
  return m_canvasList[role];
}

void ZImageWidget::validateCanvas(ECanvasRole role)
{
  std::shared_ptr<ZSliceCanvas> &canvas = m_canvasList[role];
  if (!canvas) {
    canvas = std::shared_ptr<ZSliceCanvas>(new ZSliceCanvas);
  }

  double scale = getSliceViewTransform().getScale();
  if (scale <= 1.0 ||
      (role != CANVAS_ROLE_OBJECT && role != CANVAS_ROLE_DYNAMIC_OBJECT)) {
    canvas->set(
          width(), height(), m_sliceViewTransform,
          ZSliceCanvas::ESetOption::DIFF_CLEAR);
  } else {
    int w = neutu::iround(width() * scale);
    int h = neutu::iround(height() * scale);
    ZSliceViewTransform t = m_sliceViewTransform;
    t.setScale(1.0);
    t.setAnchor(m_viewAnchorX * w, m_viewAnchorY * h);
    canvas->set(width(), height(), t, ZSliceCanvas::ESetOption::DIFF_CLEAR);
  }
}

std::shared_ptr<ZSliceCanvas> ZImageWidget::getValidCanvas(ECanvasRole role)
{
  validateCanvas(role);
  return getCanvas(role);
}

std::shared_ptr<ZSliceCanvas> ZImageWidget::getClearCanvas(ECanvasRole role)
{
  validateCanvas(role);
  getCanvas(role)->resetCanvas();

  return getCanvas(role);
}


void ZImageWidget::maximizeViewPort(const ZIntCuboid &worldRange)
{
  qDebug() << "ZImageWidget::maximizeViewPort";
//  m_viewProj.maximizeViewPort();
  m_sliceViewTransform.fitModelRange(worldRange, width(), height());
  notifyTransformChanged();
}

void ZImageWidget::enableOffsetAdjustment(bool on)
{
  m_offsetAdjustment = on;
}

QPointF ZImageWidget::getAnchorPoint() const
{
  return QPointF(width() * m_viewAnchorX, height() * m_viewAnchorY);
}

ZPoint ZImageWidget::getAnchorPoint(neutu::data3d::ESpace space) const
{
  QPointF pt = getAnchorPoint();
  return m_sliceViewTransform.transform(
        ZPoint(pt.x(), pt.y(), 0), neutu::data3d::ESpace::CANVAS, space);
}

ZAffineRect ZImageWidget::getViewPort() const
{
  return m_sliceViewTransform.inverseTransformRect(0, 0, width(), height());
}

void ZImageWidget::paintEvent(QPaintEvent * event)
{
#ifdef _DEBUG_2
  LDEBUG() << "ZImageWidget::paintEvent";
#endif

  QWidget::paintEvent(event);

#ifdef _DEBUG_2
  std::cout << "ZImageWidget::paintEvent() starts, index=" << m_paintBundle->sliceIndex() << std::endl;
#endif

  if (m_sliceViewTransform.getScale() > 0.0 && !isPaintBlocked()) {
    ZPainter painter;

#ifdef _DEBUG_2
    std::cout << "Axis: " << m_sliceAxis << std::endl;
    m_viewProj.print();
#endif

    if (!painter.begin(this)) {
      std::cout << "......failed to begin painter" << std::endl;
    }

    if (m_smoothDisplay) {
      painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
      painter.setRenderHint(QPainter::HighQualityAntialiasing, true); //not sure if this has any effect
    } else {
      painter.setRenderHint(QPainter::Antialiasing, true);
    }


    /* draw background */
//    painter.save();
    QBrush bgBrush(QColor(164,164, 164), Qt::CrossPattern);
    painter.fillRect(QRect(0, 0, screenSize().width(), screenSize().height()),
                     Qt::gray);
    painter.fillRect(QRect(0, 0, screenSize().width(), screenSize().height()),
                     bgBrush);

    for (auto canvas : m_canvasList) {
      if (canvas) {
        if (canvas->isPainted()) {
#ifdef _DEBUG_2
          canvas->save(GET_TEST_DATA_DIR + "/_test.png");
#endif
          canvas->paintTo(this, m_sliceViewTransform);
        }
      }
    }

    paintObject();

    if (m_showingZoomHint) {
      paintZoomHint();
    } else {
      paintCrossHair();
    }
  }
//    painter.restore();
//    QSize size = projectSize();

#if 0
    if (m_image != NULL) {
      painter.drawImage(m_viewProj, *m_image);
#ifdef _DEBUG_2
      m_image->save((GET_TEST_DATA_DIR + "/test.tif").c_str());
#endif
    }

    //tic();
    if (m_tileCanvas != NULL) {
#ifdef _DEBUG_2
      m_tileCanvas->save((GET_TEST_DATA_DIR + "/test.tif").c_str());
#endif
#ifdef _DEBUG_2
      qDebug() << "Paint tile:" << m_viewProj.getViewPort() << m_viewProj.getProjRect();
#endif
      painter.drawPixmap(m_viewProj, *m_tileCanvas);
//      painter.drawPixmap(m_projRegion, *m_tileCanvas, m_viewPort);
    }
    //std::cout << "paint tile canvas: " << toc() << std::endl;


    //tic();
    for (int i = 0; i < m_mask.size(); ++i) {
      if (m_mask[i] != NULL) {
        painter.drawImage(m_viewProj, *(m_mask[i]));
      }
    }
    //std::cout << "paint object canvas: " << toc() << std::endl;

    if (m_dynamicObjectCanvas != NULL) {
      if (m_dynamicObjectCanvas->isVisible()) {
        m_dynamicObjectCanvas->updateProjTransform(viewPort(), projectRegion());
#ifdef _DEBUG_2
    m_dynamicObjectCanvas->save((GET_TEST_DATA_DIR + "/test.tif").c_str());
#endif
        painter.drawPixmap(*m_dynamicObjectCanvas);
      }
    }

    /*
    if (m_widgetCanvas) {
      if (m_widgetCanvas->isVisible()) {
        painter.drawPixmap(*m_widgetCanvas);
      }
    }
    */

    //tic();
    if (m_objectCanvas != NULL) {
#ifdef _DEBUG_2
      m_objectCanvas->save((GET_TEST_DATA_DIR + "/test.tif").c_str());
#endif
      if (m_objectCanvas->isVisible()) {
        painter.drawPixmap(m_viewProj, *m_objectCanvas);
      }
    }
    //std::cout << "paint object canvas: " << toc() << std::endl;

    if (m_activeDecorationCanvas != NULL) {
      if (m_activeDecorationCanvas->isVisible()) {
#if 0
        QRectF targetRect = projectRegion();
        if (m_activeDecorationCanvas->getTransform().getSx() != 1.0) {
          targetRect.setSize(m_activeDecorationCanvas->size());
        }
        painter.drawPixmap(targetRect, *m_activeDecorationCanvas);
#endif
        painter.drawPixmap(*m_activeDecorationCanvas);
//        painter.drawPixmapNt(*m_activeDecorationCanvas);
//        painter.drawPixmap(m_projRegion, *m_activeDecorationCanvas, m_viewPort);
      }
    }

    painter.end();

    paintObject();
//    paintDynamicObject();

    if (m_showingZoomHint) {
      paintZoomHint();
    } else {
      paintCrossHair();
    }
    //std::cout << "Screen update time per frame: " << timer.elapsed() << std::endl;
  }
#endif
}


void ZImageWidget::setImage(ZImage *image)
{
  m_image = image;
  updateGeometry();
}

void ZImageWidget::setMask(ZImage *mask, int channel)
{
  if (channel >= m_mask.size()) {
    m_mask.resize(channel + 1);
  }

  m_mask[channel] = mask;

  if (m_image == NULL) {
//    QSize maskSize = getMaskSize();
//    m_viewPort.setRect(0, 0, maskSize.width(), maskSize.height());
  }
}
#if 0
void ZImageWidget::setTileCanvas(ZPixmap *canvas)
{
  m_tileCanvas = canvas;
  if (m_image == NULL) {
//    QSize maskSize = getMaskSize();
  }
}

void ZImageWidget::setObjectCanvas(ZPixmap *canvas)
{
  m_objectCanvas = canvas;
}

void ZImageWidget::setDynamicObjectCanvas(ZPixmap *canvas)
{
  m_dynamicObjectCanvas = canvas;
}

void ZImageWidget::setActiveDecorationCanvas(ZPixmap *canvas)
{
  m_activeDecorationCanvas = canvas;
}

void ZImageWidget::setViewProj(int x0, int y0, double zoom)
{
  if (m_viewProj.getX0() != x0 || m_viewProj.getY0() != y0 ||
      m_viewProj.getZoom() != zoom) {
    m_viewProj.set(x0, y0, zoom);
    updateView();

#ifdef _DEBUG_2
    std::cout << "ZImageWidget::setViewProj: ";
    m_viewProj.print();
#endif
  }
}

void ZImageWidget::setViewProj(const QPoint &pt, double zoom)
{
  setViewProj(pt.x(), pt.y(), zoom);
}
#endif

void ZImageWidget::zoomTo(const QPoint &center, int w, int h)
{
  m_sliceViewTransform.zoomToViewRect(
        center.x() - w, center.y() - h,
        center.x() + w, center.y() + h, width(), height());
//  m_viewProj.zoomTo(center, width);
  updateView();
}

void ZImageWidget::zoomTo(
    const ZPoint &pt, double w, double h, neutu::data3d::ESpace space)
{
  if (space == neutu::data3d::ESpace::CANVAS) {
    m_sliceViewTransform.setScale(
          m_sliceViewTransform.getScale() * std::min(w / width(), h / height()));
  } else {
    m_sliceViewTransform.setScale(std::min(w / width(), h / height()));
  }
  m_sliceViewTransform.setCutCenter(pt);
}

bool ZImageWidget::isBadView() const
{
  return m_sliceViewTransform.getScale() <= m_sliceViewTransform.getMinScale();
  /*
  QRectF projRect = projectRegion();

  if (projRect.isEmpty() || !projRect.intersects(m_viewProj.getWidgetRect())) {
    return true;
  } else {
    if (m_viewProj.getZoom() < m_viewProj.getMinZoomRatio() * 1.1) {
      return true;
    }
  }

  return false;
  */
}

void ZImageWidget::setSliceAxis(neutu::EAxis axis)
{
  m_sliceViewTransform.setCutPlane(axis);
  notifyTransformChanged();
}

neutu::EAxis ZImageWidget::getSliceAxis() const
{
  return m_sliceViewTransform.getSliceAxis();
}

bool ZImageWidget::isModelWithinWidget() const
{
  if (m_sliceViewTransform.getScale() > 0.0) {
    ZPoint minCorner = m_modelRange.getMinCorner().toPoint();
    ZPoint maxCorner = m_modelRange.getMaxCorner().toPoint();
    ZPoint center = (minCorner + maxCorner) / 2.0;

    center = m_sliceViewTransform.transform(center);
    ZPoint dims = m_sliceViewTransform.transformBoxSize(
          m_modelRange.getSize().toPoint());

    neutu::geom2d::Rectangle dataRange;
    dataRange.setCenter(center.getX(), center.getY(), dims.getX(), dims.getY());

    neutu::geom2d::Rectangle canvasRange;
    canvasRange.set(0, 0, neutu::geom2d::Dims(width(), height()));

    return canvasRange.intersecting(dataRange);
  }

  return false;
}

void ZImageWidget::restoreFromBadView(const ZIntCuboid &worldRange)
{
  if (isBadView()) {
    maximizeViewPort(worldRange);
  }
}

/*
void ZImageWidget::resetTransform()
{
  m_sliceViewTransform.setCutCenter(m_modelRange.getCenter().toPoint());
  maximizeViewPort(m_modelRange);
}
*/

void ZImageWidget::setViewPort(const QRect &rect)
{
  m_sliceViewTransform.zoomToViewRect(
        rect.left(), rect.top(), rect.right(), rect.bottom(),
        width(), height());;
//  m_viewProj.setViewPort(rect);
  updateView();
}

void ZImageWidget::zoom(double zoomRatio, const QPointF &ref)
{
  ZPoint fixPoint = m_sliceViewTransform.inverseTransform(ref.x(), ref.y());

  m_sliceViewTransform.setScale(zoomRatio);
  m_sliceViewTransform.translateModelViewTransform(fixPoint, ref.x(), ref.y());

//  m_viewProj.setZoomWithFixedPoint(zoomRatio, m_viewProj.mapPointBack(ref));
  updateView();
}

/*
void ZImageWidget::setView(double zoomRatio, const QPoint &zoomOffset)
{
  m_viewProj.set(zoomOffset, zoomRatio);
  updateView();
}

void ZImageWidget::setViewPortOffset(int x, int y)
{
  m_viewProj.setOffset(x, y);
  updateView();
}
*/

void ZImageWidget::setViewPortCenterQuitely(int cx, int cy)
{
  /*
  m_viewProj.setOffset(cx - (viewPort().width() - 1) / 2,
                       cy - (viewPort().height() - 1) / 2);
                       */
}


void ZImageWidget::setZoomRatio(double zoomRatio)
{
  m_sliceViewTransform.setScale(zoomRatio);

//  m_viewProj.setZoom(zoomRatio);
  updateView();
}
/*
QSizeF ZImageWidget::projectSize() const
{
  return projectRegion().size();
}

QRectF ZImageWidget::projectRegion() const
{
  return m_viewProj.getProjRect();
}


QRect ZImageWidget::viewPort() const
{
  return m_viewProj.getViewPort();
}

QRect ZImageWidget::canvasRegion() const
{
  return m_viewProj.getCanvasRect();
}
*/

#define VIEW_PORT_AREA_THRESHOLD 25000000

void ZImageWidget::increaseZoomRatio(int x, int y, bool usingRef)
{
  if (m_sliceViewTransform.getScale() < m_sliceViewTransform.getMaxScale()) {
    double newScale =
        m_sliceViewTransform.getScale() * m_sliceViewTransform.getScaleStep();
    if (usingRef) {
      m_sliceViewTransform.setScaleFixingCanvasMapped(newScale, x, y);
    } else {
      m_sliceViewTransform.setScale(newScale);
    }

    notifyTransformChanged();
  }
  /*
  if (usingRef) {
    QPointF viewPoint = m_viewProj.mapPointBack(QPointF(x, y));
    m_viewProj.increaseZoom(viewPoint.x(), viewPoint.y());
  } else {
    QPoint viewPoint = m_viewProj.getViewPort().center();
    m_viewProj.increaseZoom(viewPoint.x(), viewPoint.y());
  }
  */

//  updateView();
}

void ZImageWidget::decreaseZoomRatio(int x, int y, bool usingRef)
{
  if (m_sliceViewTransform.getScale() > m_sliceViewTransform.getMinScale()) {
    double newScale =
        m_sliceViewTransform.getScale() / m_sliceViewTransform.getScaleStep();

    if (usingRef) {
      m_sliceViewTransform.setScaleFixingCanvasMapped(newScale, x, y);
    } else {
      m_sliceViewTransform.setScale(newScale);
    }

    notifyTransformChanged();
  }
//  updateView();
}

void ZImageWidget::increaseZoomRatio()
{
  increaseZoomRatio(0, 0, false);
}

void ZImageWidget::decreaseZoomRatio()
{
  decreaseZoomRatio(0, 0, false);
}

void ZImageWidget::moveViewPort(const QPoint &src, const QPointF &dst)
{
  m_sliceViewTransform.translateModelViewTransform(
        ZPoint(src.x(), src.y(), 0), ZPoint(dst.x(), dst.y(), 0),
        ZSliceViewTransform::EPointMatchPolicy::CANVAS_TO_CANVAS);
//  m_viewProj.move(src, dst);
//  updateView();
  notifyTransformChanged();
}

void ZImageWidget::moveViewPort(int dx, int dy)
{
  m_sliceViewTransform.translateModelViewTransform(
        ZPoint(0, 0, 0), ZPoint(dx, dy, 0),
        ZSliceViewTransform::EPointMatchPolicy::CANVAS_TO_CANVAS);
//  m_viewProj.move(x, y);
//  updateView();
  notifyTransformChanged();
}

void ZImageWidget::moveViewPort(const ZPoint &src, const QPointF &dst)
{
  m_sliceViewTransform.translateModelViewTransform(src, dst.x(), dst.y());
  notifyTransformChanged();
}

void ZImageWidget::zoom(double zoomRatio)
{
  m_sliceViewTransform.setScale(zoomRatio);
//  m_viewProj.setZoom(zoomRatio);
//  updateView();
  notifyTransformChanged();
}

template<typename ZStackObjectPtr>
bool ZImageWidget::paintObjectTmpl(
    QPainter *painter, const QList<ZStackObjectPtr> &objList)
{
  neutu::data3d::DisplayConfig config;
  config.setTransform(m_sliceViewTransform);
  bool painted = false;
  for (auto obj : objList) {
    painted = obj->display(painter, config);
  }

  return painted;

#if 0
  double zoomRatio = m_viewProj.getZoom();
  ZStackObjectPainter paintHelper;

  painter.setCanvasRange(viewPort());
  painter.setRenderHints(QPainter::Antialiasing);

  QTransform transform;
  int sliceIndex = m_paintBundle->sliceIndex();
  int zOffset = m_paintBundle->getStackOffset().getZ();
  neutu::EAxis sliceAxis = m_sliceAxis;
  if (m_paintBundle->getSliceAxis() == neutu::EAxis::ARB) {
    sliceIndex = 0;
    zOffset = 0;
    painter.normalizeCanvasRange();
    sliceAxis = neutu::EAxis::Z;
    QRect viewPort = m_viewProj.getViewPort();
    transform.translate(viewPort.width() * 0.5 * zoomRatio,
                        viewPort.height() * 0.5 * zoomRatio);
    transform.scale(zoomRatio, zoomRatio);
  } else {
    transform.translate((0.5 - m_viewProj.getX0())*zoomRatio,
                        (0.5 - m_viewProj.getY0())*zoomRatio);
    transform.scale(zoomRatio, zoomRatio);
  }
  painter.setTransform(transform);
  painter.setZOffset(zOffset);

  for (auto obj : objList) {
    if (obj->getType() == ZStackObject::EType::CROSS_HAIR) {
      QPainter rawPainter(this);
      obj->display(
            &rawPainter, zstackobject::DisplayConfigBuilder().cutPlane(sliceAxis, 0));
      /*
      ZPainter rawPainter(this);
      rawPainter.setCanvasRange(QRectF(0, 0, width(), height()));
      obj->display(rawPainter, m_paintBundle->sliceIndex(),
                   ZStackObject::EDisplayStyle::NORMAL, sliceAxis);
                   */
    } else {
      paintHelper.paint(
            obj, painter, sliceIndex, m_paintBundle->displayStyle(), sliceAxis);
    }
  }
#endif
}

bool ZImageWidget::paintObject(
    QPainter *painter, const QList<std::shared_ptr<ZStackObject>> &objList)
{
  return paintObjectTmpl(painter, objList);
}

bool ZImageWidget::paintObject(QPainter *painter, const QList<ZStackObject*> &objList)
{
  return paintObjectTmpl(painter, objList);
}

bool ZImageWidget::paintWidgetCanvas(ZImage *canvas)
{
  bool painted = false;
  if (m_paintBundle && canvas) {
    QPainter painter;
    if (painter.begin(canvas)) {
      QList<std::shared_ptr<ZStackObject>> objList =
          m_paintBundle->getVisibleDynamicObjectList();
      if (m_paintBundle->getSliceAxis() == neutu::EAxis::ARB) {
//        m_paintBundle->alignToCutPlane(objList);
      }
      painted = paintObject(&painter, objList);
//      painted = painter.isPainted();
      painter.end();

#ifdef _DEBUG_2
      canvas->save((GET_TEST_DATA_DIR + "/_test.tif").c_str());
#endif
    } else {
      std::cout << "......failed to begin painter" << std::endl;
      return false;
    }
  }

  return painted;
}

ZImage *ZImageWidget::makeWidgetCanvas() const
{
  return new ZImage(size());
//  canvas->fill(Qt::transparent);
}

void ZImageWidget::updateWidgetCanvas(ZPixmap *canvas)
{
#if 0
  if (m_widgetCanvas != canvas) {
    delete m_widgetCanvas;
    m_widgetCanvas = canvas;
#ifdef _DEBUG_2
    if (m_widgetCanvas) {
      m_widgetCanvas->save((GET_TEST_DATA_DIR + "/_test.tif").c_str());
    }
#endif
    update();
  }
#endif
}

int ZImageWidget::getCutDepth() const
{
  return getSliceViewTransform().getCutCenter().getSliceCoord(getSliceAxis());
}

int ZImageWidget::getMinCutDepth() const
{
  return m_modelRange.getMinCorner().getSliceCoord(getSliceAxis());
}

int ZImageWidget::getMaxCutDepth() const
{
  return m_modelRange.getMaxCorner().getSliceCoord(getSliceAxis());
}

void ZImageWidget::adjustMinScale()
{
  ZPoint dims = m_sliceViewTransform.getModelViewTransform().transformBoxSize(
        m_modelRange.getSize().toPoint());
  m_sliceViewTransform.setMinScale(
        std::min(width() * 0.5 / dims.getX(), height() * 0.5 / dims.getY()));
}

void ZImageWidget::adjustTransformWithResize()
{
#ifdef _DEBUG_2
  std::cout << "Transform: " << m_sliceViewTransform << std::endl;
#endif
//  m_sliceViewTransform.translateModelViewTransform(
//        m_sliceViewTransform.getCutCenter(),
//        m_viewAnchorX * width(), m_viewAnchorY * height());
  m_sliceViewTransform.setAnchor(
        m_viewAnchorX * width(), m_viewAnchorY * height());
  adjustMinScale();

#ifdef _DEBUG_2
  std::cout << "Transform: " << m_sliceViewTransform << std::endl;
#endif
}


void ZImageWidget::paintDynamicObject()
{
  auto canvas = getValidCanvas(ECanvasRole::CANVAS_ROLE_WIDGET);
  QList<std::shared_ptr<ZStackObject>> objList =
      m_paintBundle->getVisibleDynamicObjectList();
  ZSliceCanvasPaintHelper p(*canvas);
  paintObject(p.getPainter(), objList);

  /*
  if (m_paintBundle) {
    ZPainter painter;
    if (m_widgetCanvas == nullptr) {
      m_widgetCanvas = new ZPixmap(size());
    }
    m_widgetCanvas->fill(Qt::transparent);

    if (!painter.begin(m_widgetCanvas)) {
      std::cout << "......failed to begin painter" << std::endl;
     return;
    }

    QList<ZStackObject*> visibleObject =
        m_paintBundle->getVisibleDynamicObjectList();
    paintObject(painter, visibleObject);

    painter.end();

    QPainter widgetPainter(this);
    widgetPainter.drawPixmap(0, 0, *m_widgetCanvas);
  }
  */
}


void ZImageWidget::paintObject()
{
  if (m_paintBundle) {
    QPainter painter;
    if (!painter.begin(this)) {
      std::cout << "......failed to begin painter" << std::endl;
      return;
    }


    QList<ZStackObject*> visibleObject = m_paintBundle->getVisibleObjectList(
          ZStackObject::ETarget::WIDGET);

    paintObject(&painter, visibleObject);
//    paintObject(painter, visibleObject);

    /*
    if (getCanvas(ECanvasRole::CANVAS_ROLE_WIDGET)) {
      getCanvas(ECanvasRole::CANVAS_ROLE_WIDGET)->paintTo(
            this, m_sliceViewTransform);
    }
    */
    /*
    if (m_widgetCanvas) {
      QPainter widgetPainter(this);
      widgetPainter.drawPixmap(0, 0, *m_widgetCanvas);
    }
    */

#if 0
    double zoomRatio = m_viewProj.getZoom();
//    double zoomRatio =  double(projectSize()).width() / m_viewPort.width();

    ZStackObjectPainter paintHelper;

    painter.setCanvasRange(viewPort());

    if (m_widgetCanvas) {
      if (m_widgetCanvas->size() != this->size()) {
        delete m_widgetCanvas;
        m_widgetCanvas = nullptr;
      }
    }

    if (m_widgetCanvas == nullptr) {
      m_widgetCanvas = new ZPixmap(size());
    }
    m_widgetCanvas->fill(Qt::transparent);

    if (!painter.begin(m_widgetCanvas)) {
      std::cout << "......failed to begin painter" << std::endl;
      return;
    }
#ifdef _DEBUG_2
    std::cout << x() - parentWidget()->x() << " "
              << y() - parentWidget()->y()
              << 0.5 - m_viewPort.x() << std::endl;
#endif
    painter.setRenderHints(QPainter::Antialiasing/* | QPainter::HighQualityAntialiasing*/);

    QTransform transform;
    transform.translate((0.5 - m_viewProj.getX0())*zoomRatio,
                        (0.5 - m_viewProj.getY0())*zoomRatio);
    transform.scale(zoomRatio, zoomRatio);
//    transform.translate(-m_paintBundle->getStackOffset().getX(),
//                        -m_paintBundle->getStackOffset().getY());
    painter.setTransform(transform);
    painter.setZOffset(m_paintBundle->getStackOffset().getZ());
    //.getSliceCoord(getSliceAxis()));



//    painter.setStackOffset(m_paintBundle->getStackOffset());
#if 0
    std::vector<const ZStackObject*> visibleObject;
    ZPaintBundle::const_iterator iter = m_paintBundle->begin();
#ifdef _DEBUG_2
    std::cout << "visible: " << std::endl;
#endif

    for (;iter != m_paintBundle->end(); ++iter) {
      const ZStackObject *obj = *iter;
      if (obj->getTarget() == ZStackObject::ETarget::WIDGET &&
          obj->isSliceVisible(m_paintBundle->getZ(), m_sliceAxis)) {
        if (obj->getSource() != ZStackObjectSourceFactory::MakeNodeAdaptorSource()) {
          visibleObject.push_back(obj);
        }
      }
    }
#endif

#ifdef _DEBUG_2
    std::cout << "---" << std::endl;
    std::cout << m_paintBundle->sliceIndex() << std::endl;
#endif
//    std::sort(visibleObject.begin(), visibleObject.end(),
//              ZStackObject::ZOrderLessThan());
    for (ZStackObject *obj : visibleObject) {
//      const ZStackObject *obj = *iter;
#ifdef _DEBUG_2
      std::cout << obj << std::endl;
#endif
      if (obj->getType() == ZStackObject::EType::CROSS_HAIR) {
        ZPainter rawPainter(this);
        rawPainter.setCanvasRange(QRectF(0, 0, width(), height()));
        obj->display(rawPainter, m_paintBundle->sliceIndex(),
                     ZStackObject::EDisplayStyle::NORMAL, m_sliceAxis);
      } else {
        paintHelper.paint(obj, painter, m_paintBundle->sliceIndex(),
                          m_paintBundle->displayStyle(), m_sliceAxis);
      }
      /*
      obj->display(painter, m_paintBundle->sliceIndex(),
                   m_paintBundle->displayStyle());
                   */
    }

//    for (iter = m_paintBundle->begin();iter != m_paintBundle->end(); ++iter) {
//      const ZStackObject *obj = *iter;
//      if (obj->getTarget() == ZStackObject::ETarget::WIDGET &&
//          obj->isSliceVisible(m_paintBundle->getZ(), m_sliceAxis)) {
//        if (obj->getSource() == ZStackObjectSourceFactory::MakeNodeAdaptorSource()) {
//          paintHelper.paint(obj, painter, m_paintBundle->sliceIndex(),
//                            m_paintBundle->displayStyle(), m_sliceAxis);
//          /*
//          obj->display(painter, m_paintBundle->sliceIndex(),
//                       m_paintBundle->displayStyle());
//                       */
//        }
//      }
//    }
#endif
    painter.end();

//    QPainter widgetPainter(this);
//    widgetPainter.drawPixmap(0, 0, *m_widgetCanvas);
  }


}

void ZImageWidget::hideZoomHint()
{
  m_showingZoomHint = false;
  update();
}

void ZImageWidget::showCrossHair(bool on)
{
  m_showingCrossHair = on;
}

void ZImageWidget::paintZoomHint()
{
  QPainter painter;
  if (!painter.begin(this)) {
    std::cout << "......failed to begin painter" << std::endl;
    return;
  }

  painter.setRenderHint(QPainter::Antialiasing, false);

  ZPoint dims = m_sliceViewTransform.transformBoxSize(
        ZPoint(m_modelRange.getWidth(), m_modelRange.getHeight(),
               m_modelRange.getDepth()));

  neutu::geom2d::Rectangle fullViewPort;
  fullViewPort.setCenter(0, 0, dims.x(), dims.y());
  fullViewPort.round();

  neutu::geom2d::Rectangle viewPort =
      m_sliceViewTransform.getViewportV(width(), height());
  viewPort.round();


  if (!viewPort.contains(fullViewPort) &&m_isViewHintVisible) {
    painter.setPen(QPen(QColor(0, 0, 255, 128)));

    /*
    double ratio = std::min(
          m_viewProj.getWidgetRect().width() * 0.2 / canvasSize().width(),
          m_viewProj.getWidgetRect().height() * 0.2 / canvasSize().height());

    //Canvas hint
    painter.drawRect(
          0, 0, ratio * canvasSize().width(), ratio * canvasSize().height());

    painter.setPen(QPen(QColor(0, 255, 0, 128)));

    //Viewport hint
    painter.drawRect(ratio * (viewPort().left() - canvasRegion().left()),
                     ratio * (viewPort().top() - canvasRegion().top()),
                     ratio * viewPort().width(), ratio * viewPort().height());
                     */
  }
}

void ZImageWidget::paintCrossHair()
{
  QPainter painter;
  if (!painter.begin(this)) {
    std::cout << "......failed to begin painter" << std::endl;
    return;
  }

  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setPen(QPen(QColor(0, 0, 255, 64)));

  /*
  int x1 = m_viewProj.getWidgetRect().right();
  int y1 = m_viewProj.getWidgetRect().bottom();

  double cx = x1 * 0.5;
  double cy = y1 * 0.5;
  */

  double ca = width() * m_viewAnchorX;
  double cb = width() * m_viewAnchorY;
  painter.drawLine(QPointF(ca, 0), QPointF(ca, height()));
  painter.drawLine(QPointF(0, cb), QPointF(width(), cb));
}

QSize ZImageWidget::minimumSizeHint() const
{
  if (m_image != NULL) {
    return QSize(std::min(300, m_image->width()),
                 std::min(300, m_image->height()));
  } else {
    return QSize(300, 300);
  }
}


QSize ZImageWidget::sizeHint() const
{
  return QSize(512, 512);
  /*
  if (projectRegion().size().isEmpty()) {
    return minimumSizeHint();
  } else {
    return QSize(std::ceil(projectRegion().width()),
                 std::ceil(projectRegion().height()));
  }
  */
}


#if 0
void ZImageWidget::resetViewProj(int x0, int y0, int w, int h)
{
  resetViewProj(x0, y0, w, h, QRect());
#if 0
#ifdef _DEBUG_2
  std::cout << "ZImageWidget::resetViewProj" << std::endl;
#endif
  setCanvasRegion(x0, y0, w, h);
  m_viewProj.setWidgetRect(rect());
  m_viewProj.maximizeViewPort();
  m_isReady = false;

  updateView();
#endif
}
#endif


/*
void ZImageWidget::resetViewProj(int x0, int y0, int w, int h, const QRect &viewPort)
{
  setCanvasRegion(x0, y0, w, h);
  m_viewProj.setWidgetRect(rect());
  if (viewPort.isValid()) {
    m_viewProj.setViewPort(viewPort);
  } else {
    m_viewProj.maximizeViewPort();
  }
//  m_isReady = true;

  updateView();
}


void ZImageWidget::setCanvasRegion(int x0, int y0, int w, int h)
{
#ifdef _DEBUG_2
  std::cout << "ZImageWidget::setCanvasRegion: " << m_sliceAxis << std::endl;
  std::cout << "  " << x0 << " " << y0 << " " << w << " " << h << std::endl;
#endif

  QRect rect(x0, y0, w, h);
  if (m_viewProj.getCanvasRect() != rect) {
    m_viewProj.setCanvasRect(QRect(x0, y0, w, h));
  }
}
*/

#if 0
bool ZImageWidget::isColorTableRequired()
{
  if (m_image != NULL) {
    if (m_image->format() == QImage::Format_Indexed8) {
      return true;
    }
  }

  return false;
}
#endif

void ZImageWidget::addColorTable()
{
  if (m_image != NULL) {
    QVector<QRgb> colorTable(256);
    for (int i = 0; i < colorTable.size(); i++) {
      colorTable[i] = qRgb(i, i, i);
    }
    m_image->setColorTable(colorTable);
  }
}

QSize ZImageWidget::screenSize() const
{
  return size();
  /*
  if (canvasSize().isEmpty()) {
    return QSize(0, 0);
  } else {
    return size();
  }
  */
}

#if 0
QSize ZImageWidget::canvasSize() const
{
  return canvasRegion().size();
  /*
  if (m_image == NULL) {
    return getMaskSize();
  } else {
    return m_image->size();
  }
  */
}


QPointF ZImageWidget::canvasCoordinate(QPoint widgetCoord) const
{
  return worldCoordinate((widgetCoord)) - canvasRegion().topLeft();
}


QPointF ZImageWidget::worldCoordinate(QPoint widgetCoord) const
{
  QSizeF csize = projectSize();
  //QSize isize = canvasSize();

  QPointF pt;

  if (csize.width() > 0 && csize.height() > 0) {
    pt.setX(static_cast<double>(widgetCoord.x() * (viewPort().width()))/
            (csize.width()) + viewPort().left() - 0.5);
    pt.setY(static_cast<double>(widgetCoord.y() * (viewPort().height()))/
            (csize.height()) + viewPort().top() - 0.5);
  }

  return pt;
}
#endif

QMenu* ZImageWidget::leftMenu()
{
  return m_leftButtonMenu;
}

QMenu* ZImageWidget::rightMenu()
{
  return m_rightButtonMenu;
}

bool ZImageWidget::popLeftMenu(const QPoint &pos)
{
  if (!m_leftButtonMenu->isEmpty()) {
    m_leftButtonMenu->popup(mapToGlobal(pos));
    return true;
  }

  return false;
}

bool ZImageWidget::popRightMenu(const QPoint &pos)
{
  if (!m_rightButtonMenu->isEmpty()) {
    m_rightButtonMenu->popup(mapToGlobal(pos));
    return true;
  }

  return false;
}

bool ZImageWidget::showContextMenu(QMenu *menu, const QPoint &pos)
{
  if (menu != NULL) {
    if (!menu->isEmpty()) {
      menu->popup(mapToGlobal(pos));
      return true;
    }
  }

  return false;
}

void ZImageWidget::mouseReleaseEvent(QMouseEvent *event)
{  
  neutu::LogMouseReleaseEvent(
        m_pressedButtons, event->modifiers(), "ZImageWidget");

//  neutu::LogMouseEvent(event, "release", "ZImageWidget");
//  KINFO << "Mouse released in ZImageWidget";

  m_pressedButtons = Qt::NoButton;

  emit mouseReleased(event);
}

void ZImageWidget::mouseMoveEvent(QMouseEvent *event)
{
//  if (event->buttons() == Qt::LeftButton) {
//    KINFO << "Mouse (left) dragged in ZImageWidget";
//  } else if (event->buttons() == Qt::RightButton) {
//    KINFO << "Mouse (right) dragged in ZImageWidget";
//  } else if (event->buttons() == (Qt::RightButton | Qt::LeftButton)) {
//    KINFO << "Mouse (right+right) dragged in ZImageWidget";
//  }

  neutu::LogMouseDragEvent(event, "ZImageWidget");

  if (!hasFocus() && m_hoverFocus) {
    setFocus();
  }
  emit mouseMoved(event);
}

void ZImageWidget::mousePressEvent(QMouseEvent *event)
{
//  KINFO << "Mouse pressed in ZImageWidget";

  neutu::LogMouseEvent(event, "press", "ZImageWidget");

  m_pressedButtons = event->buttons();

  emit mousePressed(event);
}

void ZImageWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
  neutu::LogMouseEvent(event, "double click", "ZImageWidget");
//  KINFO << "Mouse double clicked in ZImageWidget";

  emit mouseDoubleClicked(event);
}

void ZImageWidget::wheelEvent(QWheelEvent *event)
{
  neutu::LogMouseEvent(event, "ZImageWidget");
//  KINFO << "Mouse scrolled in ZImageWidget";

  /*
  int numSteps = -event->delta();
  if ((abs(numSteps) > 0) && (abs(numSteps) < 120)) {
    if (numSteps > 0) {
      numSteps = 1;
    } else {
      numSteps = -1;
    }
  } else {
    numSteps /= 120;
  }

  moveCutDepth(numSteps);
  */

  emit mouseWheelRolled(event);
}

void ZImageWidget::resizeEvent(QResizeEvent * /*event*/)
{
  LDEBUG() << "ZImageWidget::resizeEvent" << size() << isVisible();

#ifdef _DEBUG_2
  std::cout << "Transform: " << m_sliceViewTransform << std::endl;
#endif

  if (m_isReady) {
    adjustTransformWithResize();
    emit transformChanged();
  } else {
    m_isReady = true;
    resetView();
  }

//  m_sliceViewTransform.canvasAdjust(
//        width(), height(), m_viewAnchorX, m_viewAnchorY);

#ifdef _DEBUG_2
  std::cout << "Transform: " << m_sliceViewTransform << std::endl;
#endif

//  emit transformChanged();
//  m_viewProj.setWidgetRect(QRect(QPoint(0, 0), size()));

//  setValidViewPort(m_viewPort);
}

void ZImageWidget::resetView()
{
  if (width() > 0 && height() > 0) {
    adjustMinScale();
    m_sliceViewTransform.setCutCenter(m_modelRange.getCenter().toPoint());
    m_sliceViewTransform.fitModelRange(m_modelRange, width(), height());
    notifyTransformChanged();
  }
}

void ZImageWidget::setReady(bool ready)
{
  m_isReady = ready;
}

void ZImageWidget::showEvent(QShowEvent *event)
{
  LDEBUG() << "ZImageWidget::showEvent" << size() << isVisible();
  QWidget::showEvent(event);

  if (!m_isReady && isVisible()) {
    m_isReady = true;
    resetView();

#ifdef _DEBUG_2
    std::cout << "Transform: " << m_sliceViewTransform << std::endl;
#endif
//    maximizeViewPort(m_modelRange);
//    m_sliceViewTransform.fitModelRange(m_modelRange, width(), height());
//    m_viewProj.maximizeViewPort();


  }
}

void ZImageWidget::keyPressEvent(QKeyEvent *event)
{
  event->ignore();
}

bool ZImageWidget::event(QEvent *event)
{
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent *ke = (QKeyEvent*) (event);
    if (ke != NULL) {
      if (ke->key() == Qt::Key_Tab) {
        event->ignore();
        return false;
      }
    }
  }

  return QWidget::event(event);
}

/*
int ZImageWidget::getMaxZoomRatio() const
{
  int ratio = static_cast<int>(
        std::ceil(std::max(canvasSize().width()*32.0/screenSize().width(),
                           canvasSize().height()*32.0/screenSize().height())));
  return std::min(std::min(canvasSize().width(), canvasSize().height()),
                  std::max(ratio, 32));
}
*/

/*
double ZImageWidget::getAcutalZoomRatioX() const
{
  return m_viewProj.getZoom();
//  return static_cast<double>(m_projRegion.width()) / m_viewPort.width();
}
*/

/*
double ZImageWidget::getAcutalZoomRatioY() const
{
  return m_viewProj.getZoom();
//  return static_cast<double>(m_projRegion.height()) / m_viewPort.height();
}
*/

void ZImageWidget::updateView()
{
  //View port adjustment
  /*
  if (m_offsetAdjustment) {
    int zoom = neutu::iround(std::log2(getViewProj().getZoom())) + 1;
    if (zoom > 0) {
      m_viewProj.alignOffset(zgeom::GetZoomScale(zoom));
    }
  }
  */

  if (!isPaintBlocked()) {
    update();
  }
//  update(QRect(QPoint(0, 0), screenSize()));
}

QSize ZImageWidget::getMaskSize() const
{
  QSize maskSize(0, 0);

  auto canvas = getCanvas(ECanvasRole::CANVAS_ROLE_MASK);
  if (canvas) {
    maskSize.setWidth(canvas->getWidth());
    maskSize.setHeight(canvas->getHeight());
  }

  canvas = getCanvas(ECanvasRole::CANVAS_ROLE_TILE);
  if (canvas) {
    if (canvas->getWidth() > maskSize.width()) {
      maskSize.setWidth(canvas->getWidth());
    }
    if (canvas->getHeight() > maskSize.height()) {
      maskSize.setHeight(canvas->getHeight());
    }
  }

#if 0

  for (QVector<ZImage*>::const_iterator iter = m_mask.begin();
       iter != m_mask.end(); ++iter) {
    const ZImage *image = *iter;
    if (image != NULL) {
      if (image->size().width() > maskSize.width()) {
        maskSize.setWidth(image->size().width());
      }
      if (image->size().height() > maskSize.height()) {
        maskSize.setHeight(image->size().height());
      }
    }
  }

  if (m_tileCanvas != NULL) {
    if (m_tileCanvas->width() > maskSize.width()) {
      maskSize.setWidth(m_tileCanvas->width());
    }
    if (m_tileCanvas->size().height() > maskSize.height()) {
      maskSize.setHeight(m_tileCanvas->size().height());
    }
  }

#endif

  return maskSize;
}

void ZImageWidget::removeCanvas(ZImage *canvas)
{
  if (m_image == canvas) {
    setImage(NULL);
  } else {
    for (QVector<ZImage*>::iterator iter = m_mask.begin(); iter != m_mask.end();
         ++iter) {
      if (*iter == canvas) {
        *iter = NULL;
      }
    }
  }
}

/*
void ZImageWidget::removeCanvas(ZPixmap *canvas)
{
  if (m_objectCanvas == canvas) {
    setObjectCanvas(NULL);
  } else if (m_tileCanvas == canvas) {
    setTileCanvas(NULL);
  } else if (m_activeDecorationCanvas == canvas) {
    setActiveDecorationCanvas(NULL);
  } else if (m_dynamicObjectCanvas == canvas) {
    setDynamicObjectCanvas(NULL);
  }
}
*/

QSizeF ZImageWidget::getViewportSize() const
{
  return QSizeF(width() / getSliceViewTransform().getScale(),
                height() / getSliceViewTransform().getScale());
}

const ZSliceViewTransform &ZImageWidget::getSliceViewTransform() const
{
  return m_sliceViewTransform;
}

void ZImageWidget::notifyTransformChanged()
{
  if (m_isReady) {
    emit transformChanged();
  }

  emit transformControlSyncNeeded();

#ifdef _DEBUG_2
  std::cout << "Transform: " << m_sliceViewTransform << std::endl;
#endif
}

void ZImageWidget::setSliceViewTransform(const ZSliceViewTransform &t)
{
  m_sliceViewTransform = t;
  notifyTransformChanged();
}

void ZImageWidget::setCutCenter(double x, double y, double z)
{
  m_sliceViewTransform.setCutCenter(x, y, z);
  notifyTransformChanged();
}

void ZImageWidget::setCutCenter(const ZPoint &center)
{
  setCutCenter(center.getX(), center.getY(), center.getZ());
}

void ZImageWidget::setCutCenter(const ZIntPoint &center)
{
  setCutCenter(center.getX(), center.getY(), center.getZ());
}

void ZImageWidget::moveCutDepth(double dz)
{
  double z = 0.0;
  double minZ = 0.0;
  double maxZ = 0.0;
  if (getSliceAxis() == neutu::EAxis::ARB) {
    ZPoint modelCenter = m_modelRange.getExactCenter();
    ZPoint currentCenter = m_sliceViewTransform.getCutCenter();
    z = (currentCenter - modelCenter).dot(
          m_sliceViewTransform.getCutPlaneNormal());
    minZ = -m_modelRange.getDiagonalLength() / 2.0;
    maxZ = -minZ;
  } else {
    minZ = m_modelRange.getMinCorner().getSliceCoord(getSliceAxis());
    maxZ = m_modelRange.getMaxCorner().getSliceCoord(getSliceAxis());
    ZPoint currentCenter = m_sliceViewTransform.getCutCenter();
    z = currentCenter.getSliceCoord(getSliceAxis());
  }

  if (z + dz < minZ) {
    dz = minZ - z;
  } else if (z + dz > maxZ) {
    dz = maxZ - z;
  }

  if (dz != 0.0) {
    m_sliceViewTransform.moveCutDepth(dz);
    notifyTransformChanged();
  }
}

ZPoint ZImageWidget::getCutCenter() const
{
  return m_sliceViewTransform.getCutCenter();
}

ZPoint ZImageWidget::transform(
    const ZPoint &pt, neutu::data3d::ESpace src, neutu::data3d::ESpace dst) const
{
  return getSliceViewTransform().transform(pt, src, dst);
}

void ZImageWidget::recordTransform()
{
  m_prevSliceViewTransform = getSliceViewTransform();
}

void ZImageWidget::setModelRange(const ZIntCuboid &range)
{
  m_modelRange = range;
}

ZIntCuboid ZImageWidget::getModelRange() const
{
  return m_modelRange;
}

void ZImageWidget::reset()
{
  m_image = NULL;
  m_mask.clear();
//  m_objectCanvas = NULL;
//  m_tileCanvas = NULL;
//  m_activeDecorationCanvas = NULL;
//  m_dynamicObjectCanvas = NULL;

//  m_viewProj.reset();
//  m_viewPort.setSize(QSize(0, 0));
//  m_canvasRegion.setSize(QSize(0, 0));
//  m_projRegion.setSize(QSize(0, 0));
}
