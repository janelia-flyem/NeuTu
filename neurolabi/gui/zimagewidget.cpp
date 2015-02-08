#include <QtGui>
#include <cstring>
#include "tz_rastergeom.h"
#include "zimagewidget.h"
#include <cmath>
#include "zpainter.h"
#include "zpaintbundle.h"

ZImageWidget::ZImageWidget(QWidget *parent, QImage *image) : QWidget(parent),
  m_isViewHintVisible(true)
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
}

ZImageWidget::~ZImageWidget()
{
  if (m_isowner == true) {
    if (m_image != NULL) {
      delete m_image;
    }
  }
}

void ZImageWidget::setImage(QImage *image)
{
  if (image != NULL) {
    if (m_viewPort.width() == 0) {
      m_viewPort.setRect(0, 0, image->width(), image->height());
    }
  }

  m_image = image;
  updateGeometry();
  m_isowner = false;
}

void ZImageWidget::setMask(QImage *mask, int channel)
{
  if (channel >= m_mask.size()) {
    m_mask.resize(channel + 1);
  }

  m_mask[channel] = mask;
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

void ZImageWidget::setProjRegion(const QRect &rect)
{
  if (m_projRegion != rect) {
    m_projRegion = rect;
    //update(QRect(QPoint(0, 0), screenSize()));
  }
}

void ZImageWidget::setView(const QRect &viewPort, const QRect &projRegion)
{
  if ((m_viewPort != viewPort) || (m_projRegion != projRegion)) {
    m_viewPort = viewPort;
    m_projRegion = projRegion;
  }
}
#if 1
void ZImageWidget::zoom(int zoomRatio, const QPoint &ref)
{
  int dc, ec, dv, ev, dp, ep, ds, es;
  int td, te, td0, te0, cd, ce;

  QPoint vpOffset = m_viewPort.topLeft();
  QSize vpSize = m_viewPort.size();
  QSize projSize = projectSize();

  if (canvasSize().width() * screenSize().height() >=
      canvasSize().height() * screenSize().width()) {
    dc = canvasSize().width();
    ec = canvasSize().height();
    dv = vpSize.width();
    ev = vpSize.height();
    dp = projSize.width();
    ep = projSize.height();
    ds = screenSize().width();
    es = screenSize().height();
    td = vpOffset.x();
    te = vpOffset.y();
    td0 = vpOffset.x();
    te0 = vpOffset.y();
    cd = ref.x();
    ce = ref.y();
  } else {
    dc = canvasSize().height();
    ec = canvasSize().width();
    dv = vpSize.height();
    ev = vpSize.width();
    dp = projSize.height();
    ep = projSize.width();
    ds = screenSize().height();
    es = screenSize().width();
    td = vpOffset.y();
    te = vpOffset.x();
    td0 = vpOffset.y();
    te0 = vpOffset.x();
    cd = ref.y();
    ce = ref.x();
  }

  int dv0 = dv;
  int ev0 = ev;
  int ep0 = ep;

  dp = ds;
  dv = dc / zoomRatio;
  td = td0 + cd * (dv0 - dv) / (dp - 1);
  if (td < 0) {
    td = 0;
  }

  if (td > dc - dv) {
    td = dc - dv;
  }

  ep = es;
  ev = dv * es / dp;
  if (ev > ec) {
    ev = ec;
  }

  te = ce * (ev0 - 1) / (ep0 - 1) - ce * (ev - 1) / (ep - 1) + te0;
  if (te < 0) {
    te = 0;
  }

  if (ev > ec - te) {
    ev = ec - te;
  }
  ep = dp * ev / dv;

  te = ce * (ev0 - 1) / (ep0 - 1) - ce * (ev - 1) / (ep - 1) + te0;
  if (te > ec - ev) {
    te = ec - ev;
  }
  if (te < 0) {
    te = 0;
  }

  if (canvasSize().width() * screenSize().height() >=
      canvasSize().height() * screenSize().width()) {
    setView(QRect(td, te, dv, ev), QRect(0, 0, dp, ep));
  } else {
    setView(QRect(te, td, ev, dv), QRect(0, 0, ep, dp));
  }
}
#endif

void ZImageWidget::setValidViewPort(const QRect &viewPort)
{
  int dc, ec, dv, ev, dp, ep, ds, es;
  int td, te;

  //QPoint vpOffset = m_viewPort.topLeft();
  QSize vpSize = viewPort.size();
  QSize projSize = projectSize();

  if (canvasSize().width() * screenSize().height() >=
      canvasSize().height() * screenSize().width()) {
    dc = canvasSize().width();
    ec = canvasSize().height();
    dv = vpSize.width();
    ev = vpSize.height();
    dp = projSize.width();
    ep = projSize.height();
    ds = screenSize().width();
    es = screenSize().height();
    td = viewPort.left();
    te = viewPort.top();
  } else {
    dc = canvasSize().height();
    ec = canvasSize().width();
    dv = vpSize.height();
    ev = vpSize.width();
    dp = projSize.height();
    ep = projSize.width();
    ds = screenSize().height();
    es = screenSize().width();
    td = viewPort.top();
    te = viewPort.left();
  }

  dp = ds;
//  dv = dc / zoomRatio;
  ev = dv * es / dp;
  if (ev > ec - te) {
    ev = ec - te;
  }
  ep = dp * ev / dv;

  if (canvasSize().width() * screenSize().height() >=
      canvasSize().height() * screenSize().width()) {
    setView(QRect(td, te, dv, ev), QRect(0, 0, dp, ep));
  } else {
    setView(QRect(te, td, ev, dv), QRect(0, 0, ep, dp));
  }
}

void ZImageWidget::setView(int zoomRatio, const QPoint &zoomOffset)
{
#if 1
  QRect viewPort;
  viewPort.setTopLeft(zoomOffset);
  viewPort.setSize(canvasSize() / zoomRatio);

  setValidViewPort(viewPort);
#else
  int dc, ec, dv, ev, dp, ep, ds, es;
  int td, te;

  //QPoint vpOffset = m_viewPort.topLeft();
  QSize vpSize = m_viewPort.size();
  QSize projSize = projectSize();

  if (canvasSize().width() * screenSize().height() >=
      canvasSize().height() * screenSize().width()) {
    dc = canvasSize().width();
    ec = canvasSize().height();
    dv = vpSize.width();
    ev = vpSize.height();
    dp = projSize.width();
    ep = projSize.height();
    ds = screenSize().width();
    es = screenSize().height();
    td = zoomOffset.x();
    te = zoomOffset.y();
  } else {
    dc = canvasSize().height();
    ec = canvasSize().width();
    dv = vpSize.height();
    ev = vpSize.width();
    dp = projSize.height();
    ep = projSize.width();
    ds = screenSize().height();
    es = screenSize().width();
    td = zoomOffset.y();
    te = zoomOffset.x();
  }

  dp = ds;
  dv = dc / zoomRatio;
  ev = dv * es / dp;
  if (ev > ec - te) {
    ev = ec - te;
  }
  ep = dp * ev / dv;

  if (canvasSize().width() * screenSize().height() >=
      canvasSize().height() * screenSize().width()) {
    setView(QRect(td, te, dv, ev), QRect(0, 0, dp, ep));
  } else {
    setView(QRect(te, td, ev, dv), QRect(0, 0, ep, dp));
  }
#endif
}

void ZImageWidget::setViewPortOffset(int x, int y)
{
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

  setViewPort(QRect(x, y, m_viewPort.width(), m_viewPort.height()));
}

void ZImageWidget::setZoomRatio(int zoomRatio)
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

  zoomRatio = std::max(zoomRatio, 1);
  zoomRatio = std::min(getMaxZoomRatio(), zoomRatio);
  zoom(zoomRatio);
  update();
}

void ZImageWidget::increaseZoomRatio()
{
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
}

void ZImageWidget::decreaseZoomRatio()
{
  int oldWidth = m_viewPort.width();
  int oldHeight = m_viewPort.height();

  int zoomRatio = imax2(
        iround(static_cast<double>(canvasSize().width()) /m_viewPort.width()),
        iround(static_cast<double>(canvasSize().height()) / m_viewPort.height())
        );

  if (zoomRatio > 1) {
    zoom(--zoomRatio);
    if (m_viewPort.width() == oldWidth && m_viewPort.height() == oldHeight) {
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
}

void ZImageWidget::moveViewPort(int x, int y)
{
  setViewPortOffset(m_viewPort.left() + x, m_viewPort.top() + y);
}


void ZImageWidget::zoom(int zoomRatio)
{
  if (zoomRatio == 1) {
    setViewPort(QRect(0, 0, canvasSize().width(), canvasSize().height()));
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

    QPoint ref;
    ref = m_projRegion.center();

    zoom(zoomRatio, ref);
#if 0
//    QRect viewPort;
//    double r = static_cast<double>(viewPort.width()) / m_projRegion.width();
    QPoint center = m_viewPort.center();
    int w = canvasSize().width() / zoomRatio;
    int h = canvasSize().height() / zoomRatio;
    int x0 = center.x() - w / 2;
    int y0 = center.y() - h / 2;

//    setValidViewPort(viewPort);
    setView(zoomRatio, QPoint(x0, y0));
#endif
  }
}

void ZImageWidget::paintEvent(QPaintEvent * /*event*/)
{
  if (m_image != NULL && !isPaintBlocked()) {
    ZPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    //zoom(m_zoomRatio);

    //Compute real viewport and projregion
#ifdef _DEBUG_
    //setView(m_zoomRatio, m_viewPort.topLeft());
    if (m_projRegion.isEmpty() || m_viewPort.isEmpty()) {
      setView(1, QPoint(0, 0));
    }
#endif

    QSize size = projectSize();
    painter.drawImage(m_projRegion, *m_image,
                      m_viewPort);

    QRect rightRect(m_projRegion.width(), 0,
                    width() - m_projRegion.width(), height());
    painter.fillRect(rightRect, Qt::gray);

    QRect downRect(0, m_projRegion.height(),
                   width(), height() - m_projRegion.height());
    painter.fillRect(downRect, Qt::gray);

    for (int i = 0; i < m_mask.size(); ++i) {
      if (m_mask[i] != NULL) {
        painter.drawImage(QRect(0, 0, size.width(), size.height()),
                            *(m_mask[i]), m_viewPort);
      }
    }
    painter.end();
#if 1
    if (m_paintBundle) {
      double zoomRatio = size.width() * 1.0 / m_viewPort.width();
      ZPainter painter1(this);
#ifdef _DEBUG_2
      std::cout << x() - parentWidget()->x() << " "
                << y() - parentWidget()->y()
                << 0.5 - m_viewPort.x() << std::endl;
#endif
      painter1.setRenderHints(QPainter::Antialiasing/* | QPainter::HighQualityAntialiasing*/);

      QTransform transform;
      transform.translate((0.5 - m_viewPort.x())*zoomRatio,
                          (0.5 - m_viewPort.y())*zoomRatio);
      transform.scale(zoomRatio, zoomRatio);
      painter1.setTransform(transform);

      painter1.setStackOffset(m_paintBundle->getStackOffset());
      std::vector<const ZStackObject*> visibleObject;
      ZPaintBundle::const_iterator iter = m_paintBundle->begin();
#ifdef _DEBUG_2
      std::cout << "visible: " << std::endl;
#endif

      for (;iter != m_paintBundle->end(); ++iter) {
        const ZStackObject *obj = *iter;
        if (obj->getTarget() == ZStackObject::WIDGET &&
            obj->isSliceVisible(m_paintBundle->getZ())) {
          if (obj->getSource() != ZStackObject::getNodeAdapterId()) {
            visibleObject.push_back(obj);
          }
        }
      }

#ifdef _DEBUG_2
      std::cout << "---" << std::endl;
#endif
      std::sort(visibleObject.begin(), visibleObject.end(),
                ZStackObject::ZOrderCompare());
      for (std::vector<const ZStackObject*>::const_iterator
           iter = visibleObject.begin(); iter != visibleObject.end(); ++iter) {
        const ZStackObject *obj = *iter;
#ifdef _DEBUG_2
          std::cout << obj << std::endl;
#endif
        obj->display(painter1, m_paintBundle->sliceIndex(),
                     m_paintBundle->displayStyle());
      }

      for (iter = m_paintBundle->begin();iter != m_paintBundle->end(); ++iter) {
        const ZStackObject *obj = *iter;
        if (obj->getTarget() == ZStackObject::WIDGET &&
            obj->isSliceVisible(m_paintBundle->getZ())) {
          if (obj->getSource() == ZStackObject::getNodeAdapterId()) {
            obj->display(painter1, m_paintBundle->sliceIndex(),
                         m_paintBundle->displayStyle());
          }
        }
      }

      painter1.end();
    }

    painter.begin(this);
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
      painter.drawRect(ratio * m_viewPort.left(), ratio * m_viewPort.top(),
                       ratio * m_viewPort.width(), ratio * m_viewPort.height());
    }
#endif
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
  if (m_image == NULL ) {
    return minimumSizeHint();
  } else {
    return m_projRegion.size();
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
  if (m_image == NULL) {
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
  if (m_image == NULL) {
    return QSize(0, 0);
  } else {
    return m_image->size();
  }
}


QPointF ZImageWidget::canvasCoordinate(QPoint widgetCoord) const
{
  QSize csize = projectSize();
  //QSize isize = canvasSize();

  QPointF pt;

  if (csize.width() > 0 && csize.height() > 0) {
    pt.setX(static_cast<double>(widgetCoord.x() * (m_viewPort.width()))/
            (csize.width()) + m_viewPort.left() - 0.5);
    pt.setY(static_cast<double>(widgetCoord.y() * (m_viewPort.height()))/
            (csize.height()) + m_viewPort.top() - 0.5);
  }
  /*
  pt.setX(Raster_Linear_Map(widgetCoord.x(), 0, csize.width(),
                            m_viewPort.left(), m_viewPort.width()));
  pt.setY(Raster_Linear_Map(widgetCoord.y(), 0, csize.height(),
                            m_viewPort.top(), m_viewPort.height()));
*/

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
  int ratio = static_cast<int>(std::ceil(std::min(canvasSize().width()*16.0/screenSize().width(),
                                                  canvasSize().height()*16.0/screenSize().height())));
  return std::max(ratio, 16);
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
