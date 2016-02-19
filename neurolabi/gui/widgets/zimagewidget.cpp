#include <QtGui>
#include <QElapsedTimer>

#include <cstring>
#include <cmath>

#include "tz_rastergeom.h"
#include "widgets/zimagewidget.h"
#include "zpainter.h"
#include "zpaintbundle.h"
#include "neutubeconfig.h"
#include "zimage.h"
#include "zpixmap.h"
#include "zstackobjectpainter.h"

ZImageWidget::ZImageWidget(QWidget *parent, ZImage *image) : QWidget(parent),
  m_isViewHintVisible(true), m_freeMoving(false), m_hoverFocus(false)
{
  if (image != NULL) {
    m_viewPort.setRect(0, 0, image->width(), image->height());
  }

  m_projRegion.setRect(0, 0, 0, 0);

  //m_zoomRatio = 1;

  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  setAttribute(Qt::WA_OpaquePaintEvent);
  //setAttribute(Qt::WA_NoSystemBackground);
  setImage(image);
  setCursor(Qt::CrossCursor);
  setMouseTracking(true);
  m_leftButtonMenu = new QMenu(this);
  m_rightButtonMenu = new QMenu(this);
  m_paintBundle = NULL;
  m_tileCanvas = NULL;
  m_objectCanvas = NULL;
  m_activeDecorationCanvas = NULL;
  m_sliceAxis = NeuTube::Z_AXIS;
}

ZImageWidget::~ZImageWidget()
{
//  if (m_isowner == true) {
//    if (m_image != NULL) {
//      delete m_image;
//    }
//  }
}

void ZImageWidget::setImage(ZImage *image)
{
//  if (image != NULL) {
//    if (m_viewPort.width() == 0) {
//      m_viewPort.setRect(0, 0, image->width(), image->height());
//    }
//  }

  m_image = image;
  updateGeometry();
//  m_isowner = false;
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

void ZImageWidget::setActiveDecorationCanvas(ZPixmap *canvas)
{
  m_activeDecorationCanvas = canvas;
}

void ZImageWidget::setViewPort(const QRect &rect)
{
  setValidViewPort(rect);
  /*
  if (m_viewPort != rect) {
    m_viewPort = rect;
  }
  */
}

void ZImageWidget::setProjRegion(const QRectF &rect)
{
  if (m_projRegion != rect) {
    m_projRegion = rect;
    //update(QRect(QPoint(0, 0), screenSize()));
  }
}

void ZImageWidget::setView(const QRect &viewPort, const QRectF &projRegion)
{
  if ((m_viewPort != viewPort) || (m_projRegion != projRegion)) {
    m_viewPort = viewPort;
    m_projRegion = projRegion;
  }
}

void ZImageWidget::adjustViewPort(EViewPortAdjust option)
{
  m_viewPort = adjustViewPort(m_viewPort, option);
}

QRect ZImageWidget::adjustViewPort(const QRect &viewPort, EViewPortAdjust option)
{
  QRect newViewPort = viewPort;

  if (option != VIEWPORT_NO_ADJUST) {
    double wRatio = (double) viewPort.width() / screenSize().width();
    double hRatio = (double) viewPort.height() / screenSize().height();

    if (wRatio != hRatio) {
      switch (option) {
      case VIEWPORT_EXPAND:
      if (wRatio < hRatio){ //expand width
        double ratio = hRatio / wRatio;
        int width = iround(ratio * viewPort.width());
        newViewPort.setWidth(width);
      } else { // expand height
        double ratio = wRatio / hRatio;
        int height = iround(ratio * viewPort.height());
        newViewPort.setHeight(height);
      }
        break;
      case VIEWPORT_SHRINK:
      if (wRatio > hRatio){ //shrink width
        double ratio = hRatio / wRatio;
        int width = iround(ratio * viewPort.width());
        newViewPort.setWidth(width);
      } else { // shrink height
        double ratio = wRatio / hRatio;
        int height = iround(ratio * viewPort.height());
        newViewPort.setHeight(height);
      }
        break;
      default:
        break;
      }
    }
  }

  return newViewPort;
}

void ZImageWidget::maximizeViewPort()
{
  double wRatio = (double) m_viewPort.width() / projectRegion().width();
  double hRatio = (double) m_viewPort.height() / projectRegion().height();

  int wMargin = screenSize().width() - projectRegion().width();
  if (wMargin > 1) {
    int dw = std::ceil(wRatio * wMargin);
    if (dw > 0) {
      m_viewPort.setWidth(imin2(canvasSize().width(), m_viewPort.width() + dw));
      m_projRegion.setWidth(m_viewPort.width() / wRatio);
    }
  }

  int hMargin = screenSize().height() - projectRegion().height();
  if (hMargin > 1) {
    int dh = std::ceil(hRatio * hMargin);
    if (dh > 0) {
      m_viewPort.setHeight(
            imin2(canvasSize().height(), m_viewPort.height() + dh));
      m_projRegion.setHeight(m_viewPort.height() / hRatio);
    }
  }
}

void ZImageWidget::alignProjRegion(double ratio)
{
  m_projRegion.setWidth(m_viewPort.width() * ratio);
  m_projRegion.setHeight(m_viewPort.height() * ratio);
}

QRect ZImageWidget::alignViewPort(
    const QRect &viewPort, double vx, double vy, double px, double py,
    double ratio) const
{
  QRect newViewPort = viewPort;

  vx -= m_canvasRegion.left();
  vy -= m_canvasRegion.top();

  px -= projectRegion().left();
  px -= projectRegion().top();

  double dx = px / ratio;
  double dy = py / ratio;

  int x0 = int(vx - dx);
  int y0 = int(vy - dy);

  if (x0 < 0) {
    x0 = 0;
  }

  if (y0 < 0) {
    y0 = 0;
  }

  if (x0 + newViewPort.width() > canvasSize().width()) {
    x0 = canvasSize().width() - newViewPort.width();
  }

  if (y0 + newViewPort.height() > canvasSize().height()) {
    y0 = canvasSize().height() - newViewPort.height();
  }

  newViewPort.moveTo(x0 + m_canvasRegion.left(), y0 + m_canvasRegion.top());

  return newViewPort;
}


QRect ZImageWidget::alignViewPort(
    const QRect &viewPort, int vx, int vy, int px, int py) const
{
  QRect newViewPort = viewPort;

  vx -= m_canvasRegion.left();
  vy -= m_canvasRegion.top();

  double wRatio = (double) viewPort.width() / projectRegion().size().width();
  double hRatio = (double) viewPort.height() / projectRegion().size().height();

  int dx = iround(wRatio * (px - projectRegion().left()));
  int dy = iround(hRatio * (py - projectRegion().top()));

  int x0 = vx - dx;
  int y0 = vy - dy;

  if (x0 < 0) {
    x0 = 0;
  }

  if (y0 < 0) {
    y0 = 0;
  }

  if (x0 + newViewPort.width() > canvasSize().width()) {
    x0 = canvasSize().width() - newViewPort.width();
  }

  if (y0 + newViewPort.height() > canvasSize().height()) {
    y0 = canvasSize().height() - newViewPort.height();
  }

  newViewPort.moveTo(x0 + m_canvasRegion.left(), y0 + m_canvasRegion.top());

  return newViewPort;
}

void ZImageWidget::adjustProjRegion(const QRect &viewPort)
{
  double wRatio = (double) screenSize().width() / viewPort.width();
  double hRatio = (double) screenSize().height() / viewPort.height();

  m_projRegion.setTopLeft(QPointF(0, 0));
  if (screenSize().width() * viewPort.height() ==
      screenSize().height() * viewPort.height()) {
    m_projRegion.setSize(QSizeF(screenSize().width(), screenSize().height()));
  } else {
    if (wRatio > hRatio) { //height should be covered
      double ratio = hRatio;
      m_projRegion.setWidth(viewPort.width() * ratio);
      m_projRegion.setHeight(screenSize().height());
    } else {
      double ratio = wRatio;
      m_projRegion.setWidth(screenSize().width());
      m_projRegion.setHeight(viewPort.height() * ratio);
    }
  }
}

void ZImageWidget::adjustProjRegion()
{
  adjustProjRegion(m_viewPort);
}

void ZImageWidget::zoom(
    double zoomRatio, const QPointF &ref, EViewPortAdjust option)
{
  if (zoomRatio < 1.0) {
    zoomRatio = 1.0;
  }

  QRect viewPort;
  viewPort.setWidth(iround(canvasSize().width() / zoomRatio));
  viewPort.setHeight(iround(canvasSize().height() / zoomRatio));


  double wRatio = m_viewPort.width() / projectRegion().width();
  double hRatio = m_viewPort.height() / projectRegion().height();

  double vx = m_viewPort.left() + ref.x() * wRatio;
  double vy = m_viewPort.top() + ref.y() * hRatio;

  double ratio = dmin2((double) screenSize().width() / viewPort.width(),
                       (double) screenSize().height() / viewPort.height());
  setProjRegion(
        QRectF(0, 0, viewPort.width() * ratio, viewPort.height() * ratio));

  m_viewPort = alignViewPort(viewPort, iround(vx), iround(vy),
                             iround(ref.x()), iround(ref.y()));
  adjustViewPort(option);
}

void ZImageWidget::zoom(double zoomRatio, const QPointF &ref)
{
  if (zoomRatio < 1.0) {
    zoomRatio = 1.0;
  }

  QRect viewPort;
  viewPort.setWidth(iround(canvasSize().width() / zoomRatio));
  viewPort.setHeight(iround(canvasSize().height() / zoomRatio));


  double wRatio = m_viewPort.width() / projectRegion().width();
  double hRatio = m_viewPort.height() / projectRegion().height();

  double vx = m_viewPort.left() + ref.x() * wRatio;
  double vy = m_viewPort.top() + ref.y() * hRatio;

  double ratio = dmin2((double) screenSize().width() / viewPort.width(),
                       (double) screenSize().height() / viewPort.height());
  setProjRegion(
        QRectF(0, 0, viewPort.width() * ratio, viewPort.height() * ratio));

  m_viewPort = alignViewPort(viewPort, iround(vx), iround(vy),
                             iround(ref.x()), iround(ref.y()));

//  ratio = (ref.x() - projectRegion().left()) / (vx - m_viewPort.left());
//  alignProjRegion(ratio);

  maximizeViewPort();

  //setValidViewPort(viewPort);
}


void ZImageWidget::setValidViewPort(const QRect &viewPort)
{
  QRect newViewPort = viewPort;

  if (newViewPort.left() < canvasRegion().left()) {
    newViewPort.setLeft(canvasRegion().left());
  }
  if (newViewPort.top() < canvasRegion().top()) {
    newViewPort.setTop(canvasRegion().top());
  }
  if (newViewPort.right() > canvasRegion().right()) {
    newViewPort.setRight(canvasRegion().right());
  }
  if (newViewPort.bottom() > canvasRegion().bottom()) {
    newViewPort.setBottom(canvasRegion().bottom());
  }

  if (!newViewPort.isValid()) {
    newViewPort = canvasRegion();
  }


  QSize vpSize = newViewPort.size();
  double wRatio = (double) screenSize().width() / vpSize.width();
  double hRatio = (double) screenSize().height() / vpSize.height();
  double ratio = dmin2(wRatio, hRatio);

  if (wRatio < hRatio) { //height has some margin
    //Compute adjusted height
    int height = iround(screenSize().height() / wRatio);
    int margin = (height - viewPort.height()) / 2; //expand both sides
    int top = newViewPort.top() - margin;
    if (top < m_canvasRegion.top()) {
      top = m_canvasRegion.top();
    }
    newViewPort.setTop(top);

    newViewPort.setHeight(
          std::min(height,
                canvasSize().height() + m_canvasRegion.top() - newViewPort.top()));
  } else if (hRatio < wRatio) { //width has some margin
    double width = screenSize().width() / hRatio;
    double margin = (width - viewPort.width()) / 2;

    double left = newViewPort.left() - margin;
    if (left < m_canvasRegion.left()) {
      left = m_canvasRegion.left();
    }
    newViewPort.setLeft(left);

    newViewPort.setWidth(
          imin2(width,
                canvasSize().width() + m_canvasRegion.left() - newViewPort.left()));
  }

  QRectF projRect = QRectF(
        0, 0, ratio * newViewPort.width(), ratio * newViewPort.height());

  setView(newViewPort, projRect);
}

void ZImageWidget::setView(double zoomRatio, const QPoint &zoomOffset)
{
  QRect viewPort;
  viewPort.setTopLeft(zoomOffset);
  viewPort.setSize(QSize(iround(canvasSize().width() / zoomRatio),
                   iround(canvasSize().height() / zoomRatio)));

  setValidViewPort(viewPort);
}

void ZImageWidget::setViewPortOffset(int x, int y)
{
  x -= m_canvasRegion.left();
  y -= m_canvasRegion.top();

  if (!m_freeMoving) {
    if (x < 0) {
      x = 0;
    }

    if (y < 0) {
      y = 0;
    }

    if (x + m_viewPort.width() > canvasSize().width()) {
      x = canvasSize().width() - m_viewPort.width();
    }

    if (y + m_viewPort.height() > canvasSize().height()) {
      y = canvasSize().height() - m_viewPort.height();
    }
  }

  setViewPort(QRect(x + m_canvasRegion.left(),
                    y + m_canvasRegion.top(),
                    m_viewPort.width(), m_viewPort.height()));
}

void ZImageWidget::setZoomRatio(double zoomRatio)
{
#if 0
  zoomRatio = std::max(zoomRatio, 1);
  zoomRatio = std::min(getMaxZoomRatio(), zoomRatio);
  if (zoomRatio != m_zoomRatio) {
    m_zoomRatio = zoomRatio;
    zoom(m_zoomRatio);
    update();
  }
#endif

  zoomRatio = std::max(zoomRatio, 1.0);
  zoomRatio = std::min(double(getMaxZoomRatio()), zoomRatio);
  zoom(zoomRatio);
  update();
}

void ZImageWidget::zoomWithWidthAligned(int x0, int x1, int cy)
{
  m_viewPort.setLeft(x0);
  m_viewPort.setRight(x1);

  double ratio = (double) m_viewPort.width() / screenSize().width();
  int height = iround(ratio * screenSize().height());
  m_viewPort.setTop(cy - (height - 1) / 2);
  m_viewPort.setHeight(height);

  adjustProjRegion();
}

void ZImageWidget::zoomWithWidthAligned(int x0, int x1, double pw, int cy)
{
  m_viewPort.setLeft(x0);
  m_viewPort.setRight(x1);

  m_projRegion.setLeft(0);
  m_projRegion.setWidth(pw);

  double ratio = m_viewPort.width() / m_projRegion.width();

  int height = iround(ratio * screenSize().height());
  m_viewPort.setTop(cy - (height - 1) / 2);
  if (m_viewPort.top() < m_canvasRegion.top()) {
    m_viewPort.setTop(m_canvasRegion.top());
  }
  m_viewPort.setHeight(height);
  if (m_viewPort.bottom() > m_canvasRegion.bottom()) {
    int bottom = m_viewPort.bottom();
    if (m_viewPort.top() > m_canvasRegion.top()) {
      int offset = std::min(m_viewPort.top() - m_canvasRegion.top(),
                            m_viewPort.bottom() - m_canvasRegion.bottom());
      m_viewPort.setTop(m_viewPort.top() - offset);
      bottom -= offset;
    }
    if (bottom > m_canvasRegion.bottom()) {
      bottom = m_canvasRegion.bottom();
    }

    m_viewPort.setBottom(bottom);
  }

  m_projRegion.setTop(0);
  m_projRegion.setHeight(m_viewPort.height() / ratio);

  update();
}

void ZImageWidget::zoomWithHeightAligned(int y0, int y1, double ph, int cx)
{
  m_viewPort.setTop(y0);
  m_viewPort.setBottom(y1);

  m_projRegion.setTop(0);
  m_projRegion.setHeight(ph);

  double ratio = m_viewPort.height() / m_projRegion.height();

  int width = iround(ratio * screenSize().width());
  m_viewPort.setLeft(cx - (width - 1) / 2);
  if (m_viewPort.left() < m_canvasRegion.left()) {
    m_viewPort.setLeft(m_canvasRegion.left());
  }
  m_viewPort.setWidth(width);
  if (m_viewPort.right() > m_canvasRegion.right()) {
    int right = m_viewPort.right();
    if (m_viewPort.left() > m_canvasRegion.left()) {
      int offset = std::min(m_viewPort.left() - m_canvasRegion.left(),
                            m_viewPort.right() - m_canvasRegion.right());
      m_viewPort.setLeft(m_viewPort.left() - offset);
      right -= offset;
    }
    if (right > m_canvasRegion.right()) {
      right = m_canvasRegion.right();
    }

    m_viewPort.setRight(right);
  }

  m_projRegion.setLeft(0);
  m_projRegion.setWidth(m_viewPort.width() / ratio);

  update();
}

#define VIEW_PORT_AREA_THRESHOLD 25000000

void ZImageWidget::increaseZoomRatio(int x, int y, bool usingRef)
{
  double zoomRatio = std::max(
        static_cast<double>(canvasSize().width()) /m_viewPort.width(),
        static_cast<double>(canvasSize().height()) / m_viewPort.height());



  if (zoomRatio < getMaxZoomRatio()) {
    int currentViewArea = m_viewPort.width() * m_viewPort.height();
    if (currentViewArea > VIEW_PORT_AREA_THRESHOLD) {
      zoomRatio *= 1.1;
    } else {
      zoomRatio *= 1.1;
    }

    if (usingRef) {
      zoom(zoomRatio, QPoint(x, y));
    } else {
      zoom(zoomRatio);
    }
    update();
  }
}

void ZImageWidget::decreaseZoomRatio(int x, int y, bool usingRef)
{
//  int oldWidth = m_viewPort.width();
//  int oldHeight = m_viewPort.height();

  double zoomRatio = std::max(
        static_cast<double>(canvasSize().width()) /m_viewPort.width(),
        static_cast<double>(canvasSize().height()) / m_viewPort.height());
//  double oldZoomRatio = zoomRatio;
  if (zoomRatio > 1) {
    int currentViewArea = m_viewPort.width() * m_viewPort.height();
    if (currentViewArea > VIEW_PORT_AREA_THRESHOLD) {
      zoomRatio /= 1.1;
    } else {
      zoomRatio /= 1.1;
    }

    if (zoomRatio < 1.0) {
      zoomRatio = 1.0;
    }

    if (usingRef) {
      zoom(zoomRatio, QPoint(x, y));
    } else {
      zoom(zoomRatio);
    }
//    m_viewPort.width() == oldWidth && m_viewPort.height() == oldHeight;
#if 0
    double currentZoomRatio = imax2(
            static_cast<double>(canvasSize().width()) /m_viewPort.width(),
            static_cast<double>(canvasSize().height()) / m_viewPort.height());

    while (oldZoomRatio == currentZoomRatio) {
      if (usingRef) {
        zoom(--zoomRatio, QPoint(x, y));
      } else {
        zoom(--zoomRatio);
      }
      currentZoomRatio = imax2(
              iround(static_cast<double>(canvasSize().width()) /m_viewPort.width()),
              iround(static_cast<double>(canvasSize().height()) / m_viewPort.height())
              );
    }
#endif

    update();
  }
}

void ZImageWidget::increaseZoomRatio()
{
  increaseZoomRatio(0, 0, false);

  /*
#if 1
  int zoomRatio = imax2(
        iround(static_cast<double>(canvasSize().width()) /m_viewPort.width()),
        iround(static_cast<double>(canvasSize().height()) / m_viewPort.height())
        );

  if (zoomRatio < getMaxZoomRatio()) {
    zoomRatio += 1;
    zoom(zoomRatio);

    update();
  }
#else
  if (m_zoomRatio < getMaxZoomRatio()) {
    m_zoomRatio += 1;
    zoom(m_zoomRatio);

#ifdef _DEBUG_2
    m_zoomRatio = 1;
#endif

    update();
  }
#endif
*/
}

void ZImageWidget::decreaseZoomRatio()
{
  decreaseZoomRatio(0, 0, false);
  /*
  int oldWidth = m_viewPort.width();
  int oldHeight = m_viewPort.height();

  int zoomRatio = imax2(
        iround(static_cast<double>(canvasSize().width()) /m_viewPort.width()),
        iround(static_cast<double>(canvasSize().height()) / m_viewPort.height())
        );

  if (zoomRatio > 1) {
    zoom(--zoomRatio);
    while (m_viewPort.width() == oldWidth && m_viewPort.height() == oldHeight) {
      zoom(--zoomRatio);
    }

    update();
  }

#if 0
  if (m_zoomRatio > 1) {
    m_zoomRatio -= 1;
    zoom(m_zoomRatio);
    update();
  }
#endif
*/
}

void ZImageWidget::moveViewPort(int x, int y)
{
  setViewPortOffset(m_viewPort.left() + x, m_viewPort.top() + y);
}

void ZImageWidget::zoom(double zoomRatio)
{
  if (zoomRatio <= 1.0) {
    setViewPort(m_canvasRegion);
    if (canvasSize().width() * screenSize().height() >=
        canvasSize().height() * screenSize().width()) {
      setProjRegion(QRect(0, 0, screenSize().width(),
                          screenSize().width() * canvasSize().height() /
                          canvasSize().width()));
    } else {
      setProjRegion(QRect(0, 0,
                          screenSize().height() * canvasSize().width() /
                          canvasSize().height(),
                          screenSize().height()));
    }
  } else {
    QPointF ref;
    ref = m_projRegion.center();

    zoom(zoomRatio, ref);
  }
}

void ZImageWidget::zoom(double zoomRatio, EViewPortAdjust option)
{
  if (zoomRatio <= 1.0) {
    zoom(zoomRatio);
  } else {
    QPointF ref;
    ref = m_projRegion.center();

    zoom(zoomRatio, ref, option);
  }
}

void ZImageWidget::paintObject()
{
  if (m_paintBundle) {
    double zoomRatio = projectSize().width() * 1.0 / m_viewPort.width();
    ZPainter painter;
    ZStackObjectPainter paintHelper;

    painter.setRange(viewPort());

    if (!painter.begin(this)) {
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
    transform.translate((0.5 - m_viewPort.left())*zoomRatio,
                        (0.5 - m_viewPort.top())*zoomRatio);
    transform.scale(zoomRatio, zoomRatio);
//    transform.translate(-m_paintBundle->getStackOffset().getX(),
//                        -m_paintBundle->getStackOffset().getY());
    painter.setTransform(transform);
    painter.setZOffset(
          m_paintBundle->getStackOffset().getSliceCoord(getSliceAxis()));

//    painter.setStackOffset(m_paintBundle->getStackOffset());
    std::vector<const ZStackObject*> visibleObject;
    ZPaintBundle::const_iterator iter = m_paintBundle->begin();
#ifdef _DEBUG_2
    std::cout << "visible: " << std::endl;
#endif

    for (;iter != m_paintBundle->end(); ++iter) {
      const ZStackObject *obj = *iter;
      if (obj->getTarget() == ZStackObject::TARGET_WIDGET &&
          obj->isSliceVisible(m_paintBundle->getZ(), m_sliceAxis)) {
        if (obj->getSource() != ZStackObjectSourceFactory::MakeNodeAdaptorSource()) {
          visibleObject.push_back(obj);
        }
      }
    }

#ifdef _DEBUG_2
    std::cout << "---" << std::endl;
    std::cout << m_paintBundle->sliceIndex() << std::endl;
#endif
    std::sort(visibleObject.begin(), visibleObject.end(),
              ZStackObject::ZOrderLessThan());
    for (std::vector<const ZStackObject*>::const_iterator
         iter = visibleObject.begin(); iter != visibleObject.end(); ++iter) {
      const ZStackObject *obj = *iter;
#ifdef _DEBUG_2
      std::cout << obj << std::endl;
#endif
      paintHelper.paint(obj, painter, m_paintBundle->sliceIndex(),
                        m_paintBundle->displayStyle(), m_sliceAxis);
      /*
      obj->display(painter, m_paintBundle->sliceIndex(),
                   m_paintBundle->displayStyle());
                   */
    }

    for (iter = m_paintBundle->begin();iter != m_paintBundle->end(); ++iter) {
      const ZStackObject *obj = *iter;
      if (obj->getTarget() == ZStackObject::TARGET_WIDGET &&
          obj->isSliceVisible(m_paintBundle->getZ(), m_sliceAxis)) {
        if (obj->getSource() == ZStackObjectSourceFactory::MakeNodeAdaptorSource()) {
          paintHelper.paint(obj, painter, m_paintBundle->sliceIndex(),
                            m_paintBundle->displayStyle(), m_sliceAxis);
          /*
          obj->display(painter, m_paintBundle->sliceIndex(),
                       m_paintBundle->displayStyle());
                       */
        }
      }
    }

    painter.end();
  }


}

void ZImageWidget::paintZoomHint()
{
  QPainter painter;
  if (!painter.begin(this)) {
    std::cout << "......failed to begin painter" << std::endl;
    return;
  }

  painter.setRenderHint(QPainter::Antialiasing, false);
  //if (m_zoomRatio > 1 && m_isViewHintVisible) {
  if ((m_viewPort.size().width() < canvasSize().width()
       || m_viewPort.size().height() < canvasSize().height()) &&
      m_isViewHintVisible) {
    painter.setPen(QPen(QColor(0, 0, 255, 128)));
    double ratio = (double) projectSize().width() /
        canvasSize().width() / 5.0;
    painter.drawRect(0, 0, ratio * canvasSize().width(),
                     ratio * canvasSize().height());
    painter.setPen(QPen(QColor(0, 255, 0, 128)));
    painter.drawRect(ratio * (m_viewPort.left() - m_canvasRegion.left()),
                     ratio * (m_viewPort.top() - m_canvasRegion.top()),
                     ratio * m_viewPort.width(), ratio * m_viewPort.height());
  }
}

void ZImageWidget::paintEvent(QPaintEvent * /*event*/)
{
#ifdef _DEBUG_2
  std::cout << "ZImageWidget::paintEvent() starts, index=" << m_paintBundle->sliceIndex() << std::endl;
#endif

  if (!canvasSize().isEmpty() && !isPaintBlocked()) {
    ZPainter painter;

    if (!painter.begin(this)) {
      std::cout << "......failed to begin painter" << std::endl;
    }

    painter.setRenderHint(QPainter::Antialiasing, true);
//    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    //Compute real viewport and projregion
//#ifdef _DEBUG_
    //setView(m_zoomRatio, m_viewPort.topLeft());
    if (m_projRegion.isEmpty() || m_viewPort.isEmpty()) {
      setView(1, QPoint(0, 0));
    }
//#endif

    /* draw gray regions */
    painter.fillRect(QRect(0, 0, screenSize().width(), screenSize().height()),
                     Qt::gray);
//    QSize size = projectSize();

    if (m_image != NULL) {
      painter.drawImage(m_projRegion, *m_image, m_viewPort);
#ifdef _DEBUG_2
      m_image->save((GET_TEST_DATA_DIR + "/test.tif").c_str());
#endif
    }

    //tic();
    if (m_tileCanvas != NULL) {
#ifdef _DEBUG_2
      m_tileCanvas->save((GET_TEST_DATA_DIR + "/test.tif").c_str());
#endif
      painter.drawPixmap(m_projRegion, *m_tileCanvas, m_viewPort);
    }
    //std::cout << "paint tile canvas: " << toc() << std::endl;


    //tic();
    for (int i = 0; i < m_mask.size(); ++i) {
      if (m_mask[i] != NULL) {
        painter.drawImage(//QRect(0, 0, size.width(), size.height()),
                          m_projRegion, *(m_mask[i]), m_viewPort);
      }
    }
    //std::cout << "paint object canvas: " << toc() << std::endl;

    //tic();
    if (m_objectCanvas != NULL) {
#ifdef _DEBUG_2
      m_objectCanvas->save((GET_TEST_DATA_DIR + "/test.tif").c_str());
#endif
      if (m_objectCanvas->isVisible()) {
        painter.drawPixmap(m_projRegion, *m_objectCanvas, m_viewPort);
      }
    }
    //std::cout << "paint object canvas: " << toc() << std::endl;

    if (m_activeDecorationCanvas != NULL) {
      if (m_activeDecorationCanvas->isVisible()) {
        painter.drawPixmap(m_projRegion, *m_activeDecorationCanvas, m_viewPort);
      }
    }

    painter.end();

    paintObject();
    paintZoomHint();
    //std::cout << "Screen update time per frame: " << timer.elapsed() << std::endl;
  }
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
  if (m_projRegion.size().isEmpty()) {
    return minimumSizeHint();
  } else {
    return QSize(std::ceil(m_projRegion.width()),
                 std::ceil(m_projRegion.height()));
  }
}

void ZImageWidget::setCanvasRegion(int x0, int y0, int w, int h)
{
  if ((m_canvasRegion.left() != x0) || (m_canvasRegion.top() != y0) ||
      (m_canvasRegion.width() != w) || (m_canvasRegion.height() != h)) {
    m_canvasRegion.setLeft(x0);
    m_canvasRegion.setTop(y0);
    m_canvasRegion.setWidth(w);
    m_canvasRegion.setHeight(h);
    m_viewPort.setSize(QSize(0, 0));
    m_projRegion.setSize(QSize(0, 0));
  }

  if (m_viewPort.width() == 0) {
    m_viewPort = m_canvasRegion;
  }
}

bool ZImageWidget::isColorTableRequired()
{
  if (m_image != NULL) {
    if (m_image->format() == QImage::Format_Indexed8) {
      return true;
    }
  }

  return false;
}

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
  if (canvasSize().isEmpty()) {
    return QSize(0, 0);
  } else {
    /*
    int width;
    int height;
    Raster_Ratio_Scale(m_image->width(), m_image->height(),
                       this->width(), this->height(), &width, &height);

    return QSize(width, height);
    */
    return size();
  }
}

QSize ZImageWidget::canvasSize() const
{
  return m_canvasRegion.size();
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
    pt.setX(static_cast<double>(widgetCoord.x() * (m_viewPort.width()))/
            (csize.width()) + m_viewPort.left() - 0.5);
    pt.setY(static_cast<double>(widgetCoord.y() * (m_viewPort.height()))/
            (csize.height()) + m_viewPort.top() - 0.5);
  }

  return pt;
}

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
  emit mouseReleased(event);
}

void ZImageWidget::mouseMoveEvent(QMouseEvent *event)
{
  if (!hasFocus() && m_hoverFocus) {
    setFocus();
  }
  emit mouseMoved(event);
}

void ZImageWidget::mousePressEvent(QMouseEvent *event)
{
  emit mousePressed(event);
}

void ZImageWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
  emit mouseDoubleClicked(event);
}

void ZImageWidget::wheelEvent(QWheelEvent *event)
{
  emit mouseWheelRolled(event);
}

void ZImageWidget::resizeEvent(QResizeEvent */*event*/)
{
  setValidViewPort(m_viewPort);
}

int ZImageWidget::getMaxZoomRatio() const
{
  int ratio = static_cast<int>(
        std::ceil(std::max(canvasSize().width()*32.0/screenSize().width(),
                           canvasSize().height()*32.0/screenSize().height())));
  return std::min(std::min(canvasSize().width(), canvasSize().height()),
                  std::max(ratio, 32));
}


double ZImageWidget::getAcutalZoomRatioX() const
{
  return static_cast<double>(m_projRegion.width()) / m_viewPort.width();
}

double ZImageWidget::getAcutalZoomRatioY() const
{
  return static_cast<double>(m_projRegion.height()) / m_viewPort.height();
}

double ZImageWidget::getActualOffsetX() const
{
  return static_cast<double>(
        m_viewPort.left() * m_projRegion.right() -
        m_viewPort.right() * m_projRegion.left()) / m_viewPort.width();
}

double ZImageWidget::getActualOffsetY() const
{
  return static_cast<double>(
        m_viewPort.top() * m_projRegion.bottom() -
        m_viewPort.bottom() * m_projRegion.top()) / m_viewPort.height();
}

void ZImageWidget::updateView()
{
  update(QRect(QPoint(0, 0), screenSize()));
}

QSize ZImageWidget::getMaskSize() const
{
  QSize maskSize(0, 0);
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

void ZImageWidget::removeCanvas(ZPixmap *canvas)
{
  if (m_objectCanvas == canvas) {
    setObjectCanvas(NULL);
  } else if (m_tileCanvas == canvas) {
    setTileCanvas(NULL);
  } else if (m_activeDecorationCanvas == canvas) {
    setActiveDecorationCanvas(NULL);
  }
}

void ZImageWidget::reset()
{
  m_image = NULL;
  m_mask.clear();
  m_objectCanvas = NULL;
  m_tileCanvas = NULL;
  m_activeDecorationCanvas = NULL;

  m_viewPort.setSize(QSize(0, 0));
  m_canvasRegion.setSize(QSize(0, 0));
  m_projRegion.setSize(QSize(0, 0));
}
