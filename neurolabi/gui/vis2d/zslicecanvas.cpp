#include "zslicecanvas.h"

#include <QDebug>

#include "common/math.h"

ZSliceCanvas::ZSliceCanvas()
{

}

ZSliceCanvas::ZSliceCanvas(int width, int height)
{
  m_pixmap = QPixmap(width, height);
  m_pixmap.fill(Qt::transparent);
  setCanvasStatus(ECanvasStatus::CLEAN);
}

void ZSliceCanvas::clear(const QColor &color)
{
  m_pixmap.fill(color);
}

void ZSliceCanvas::setCanvasStatus(ECanvasStatus status)
{
  m_status = status;
}

void ZSliceCanvas::resetCanvas()
{
  if (m_status != ECanvasStatus::CLEAN) {
    m_pixmap.fill(Qt::transparent);
    setCanvasStatus(ECanvasStatus::CLEAN);
#ifdef _DEBUG_2
    std::cout << "Canvas cleared" << std::endl;
#endif
  }
}

void ZSliceCanvas::resetCanvas(int width, int height)
{
  if (m_pixmap.width() != width || m_pixmap.height() != height) {
    m_pixmap = QPixmap(width, height);
    setCanvasStatus(ECanvasStatus::RAW);
  }
  resetCanvas();
}

void ZSliceCanvas::resetCanvas(
    int width, int height, const ZSliceViewTransform &transform)
{
  m_transform = transform;
  resetCanvas(width, height);
}

void ZSliceCanvas::set(
    int width, int height, ESetOption option)
{
  bool changed = false;
  if (m_pixmap.width() != width || m_pixmap.height() != height) {
    m_pixmap = QPixmap(width, height);
    setCanvasStatus(ECanvasStatus::RAW);
    changed = true;
  }

  bool resetNeeded = false;
  if (option == ESetOption::DIFF_CLEAR) {
    resetNeeded = changed;
  } else if (option == ESetOption::FORCE_CLEAR) {
    resetNeeded = true;
  }

  if (resetNeeded) {
    resetCanvas();
  }
}

void ZSliceCanvas::set(
    int width, int height, const ZSliceViewTransform &transform, ESetOption option)
{
  bool changed = false;
  if (m_pixmap.width() != width || m_pixmap.height() != height) {
    m_pixmap = QPixmap(width, height);
    setCanvasStatus(ECanvasStatus::RAW);
    changed = true;
  }

  if (m_transform != transform) {
    m_transform = transform;
    changed = true;
  }

  bool resetNeeded = false;
  if (option == ESetOption::DIFF_CLEAR) {
    resetNeeded = changed;
  } else if (option == ESetOption::FORCE_CLEAR) {
    resetNeeded = true;
  }

  if (resetNeeded) {
    resetCanvas();
  }
}

void ZSliceCanvas::fitTarget(int width, int height)
{
  if (m_transform.getScale() > 1.5) {
    width = neutu::iceil(width / m_transform.getScale());
    height = neutu::iceil(height / m_transform.getScale());
  }

  set(width, height, ESetOption::DIFF_CLEAR);
}

void ZSliceCanvas::setTransform(const ZSliceViewTransform &transform)
{
  m_transform = transform;
}

ZSliceViewTransform ZSliceCanvas::getTransform() const
{
  return m_transform;
}

void ZSliceCanvas::beginPainter(QPainter *painter)
{
  painter->begin(&m_pixmap);
}

/*
bool ZSliceCanvas::paintTo(
    QPaintDevice *device, const ZSliceViewTransform &&painterTransform) const
{
  return paintTo(device, painterTransform);
}
*/

bool ZSliceCanvas::paintTo(
    QPaintDevice *device, const ZSliceViewTransform &painterTransform) const
{
  bool paintable = painterTransform.hasSamePlane(m_transform);

  if (paintable) {
    QTransform transform;
    double s2 = painterTransform.getViewCanvasTransform().getScale();
    double s1 = m_transform.getScale();
    double s = s2 / s1;
    ZPoint dc = m_transform.getModelViewTransform().getCutCenter() -
        painterTransform.getModelViewTransform().getCutCenter();
    double du = dc.dot(
          m_transform.getModelViewTransform().getCutPlane().getV1()) * s2;
    double dv = dc.dot(
          m_transform.getModelViewTransform().getCutPlane().getV2()) * s2;
    double tu =   painterTransform.getViewCanvasTransform().getTx() + du -
        s * m_transform.getViewCanvasTransform().getTx();
    double tv =  painterTransform.getViewCanvasTransform().getTy() + dv -
        s * m_transform.getViewCanvasTransform().getTy();
    transform.setMatrix(s, 0, 0, 0, s, 0, tu, tv, 1);

    QPainter painter(device);
    painter.setTransform(transform);
#ifdef _DEBUG_2
    qDebug() << "painter transform:" << transform;
#endif
    painter.drawPixmap(0, 0, m_pixmap);
    return true;
  }

  return false;
}

bool ZSliceCanvas::paintTo(
    QPainter *painter, const ZSliceViewTransform &painterTransform) const
{
  bool paintable = true;
  if (painterTransform.getSliceAxis() != m_transform.getSliceAxis()) {
    paintable = false;
  } else {
    if (painterTransform.getSliceAxis() == neutu::EAxis::ARB) {
      if (!painterTransform.getCutPlaneNormal().approxEquals(
            m_transform.getCutPlaneNormal())) {
        paintable = false;
      }
    }
  }

  if (paintable) {
    QTransform transform;
    double s2 = painterTransform.getViewCanvasTransform().getScale();
    double s1 = m_transform.getScale();
    double s = s2 / s1;
    ZPoint dc = m_transform.getModelViewTransform().getCutCenter() -
        painterTransform.getModelViewTransform().getCutCenter();
    double du = dc.dot(
          m_transform.getModelViewTransform().getCutPlane().getV1()) * s2;
    double dv = dc.dot(
          m_transform.getModelViewTransform().getCutPlane().getV2()) * s2;
    double tu =   painterTransform.getViewCanvasTransform().getTx() + du -
        s * m_transform.getViewCanvasTransform().getTx();
    double tv =  painterTransform.getViewCanvasTransform().getTy() + dv -
        s * m_transform.getViewCanvasTransform().getTy();
    transform.setMatrix(s, 0, 0, 0, s, 0, tu, tv, 1);

    painter->save();
    painter->setTransform(transform, true);
#ifdef _DEBUG_2
    qDebug() << "painter transform:" << painter->transform();
#endif
    painter->drawPixmap(0, 0, m_pixmap);
    painter->restore();
    return true;
  }

  return false;
}

void ZSliceCanvas::setPainted(bool painted)
{
  if (painted) {
    setCanvasStatus(ECanvasStatus::PAINTED);
  } else {
    setCanvasStatus(ECanvasStatus::CLEAN);
  }
}

void ZSliceCanvas::setVisible(bool visible)
{
  m_isVisible = visible;
}

bool ZSliceCanvas::isPainted() const
{
  return m_status == ECanvasStatus::PAINTED;
}

bool ZSliceCanvas::isVisible() const
{
  return m_isVisible;
}

QSize ZSliceCanvas::getSize() const
{
  return m_pixmap.size();
}

int ZSliceCanvas::getWidth() const
{
  return m_pixmap.width();
}

int ZSliceCanvas::getHeight() const
{
  return m_pixmap.height();
}

QImage ZSliceCanvas::toImage() const
{
  return m_pixmap.toImage();
}

void ZSliceCanvas::fromImage(const QImage &image)
{
  m_pixmap.convertFromImage(image);
  if (!image.isNull()) {
    setPainted(true);
  }
}

void ZSliceCanvas::fromImage(const QImage &image, double u0, double v0)
{
  m_transform.translateViewCanvas(u0, v0, 0, 0);
  fromImage(image);
}

const QPixmap& ZSliceCanvas::getPixmap() const
{
  return m_pixmap;
}

QPixmap* ZSliceCanvas::getPixmapRef()
{
  return &m_pixmap;
}

void ZSliceCanvas::save(const char *filePath) const
{
  save(QString(filePath));
}

void ZSliceCanvas::save(const std::string &filePath) const
{
  save(QString::fromStdString(filePath));
}

void ZSliceCanvas::save(const QString &filePath) const
{
  m_pixmap.save(filePath);
}

/////

ZSliceCanvasPaintHelper::ZSliceCanvasPaintHelper(ZSliceCanvas &canvas)
{
  canvas.beginPainter(&m_painter);
  m_slicePainter.setModelViewTransform(
        canvas.m_transform.getModelViewTransform());
  m_slicePainter.setViewCanvasTransform(
        canvas.m_transform.getViewCanvasTransform());
}

QPainter* ZSliceCanvasPaintHelper::getPainter()
{
  return &m_painter;
}

ZSlice3dPainter* ZSliceCanvasPaintHelper::getSlicePainter()
{
  return &m_slicePainter;
}

const ZSlice3dPainter* ZSliceCanvasPaintHelper::getSlicePainter() const
{
  return &m_slicePainter;
}

void ZSliceCanvasPaintHelper::drawBall(double cx, double cy, double cz, double r,
    double depthScale, double fadingFactor)
{
  getSlicePainter()->drawBall(
        getPainter(), cx, cy, cz, r, depthScale, fadingFactor);
}

void ZSliceCanvasPaintHelper::drawBall(const ZPoint &center, double r,
    double depthScale, double fadingFactor)
{
  getSlicePainter()->drawBall(getPainter(), center, r, depthScale, fadingFactor);
}

void ZSliceCanvasPaintHelper::drawBoundBox(
    double cx, double cy, double cz, double r, double depthScale)
{
  getSlicePainter()->drawBoundBox(getPainter(), cx, cy, cz, r, depthScale);
}

void ZSliceCanvasPaintHelper::drawBoundBox(
    const ZPoint &center, double r, double depthScale)
{
  getSlicePainter()->drawBoundBox(getPainter(), center, r, depthScale);
}

void ZSliceCanvasPaintHelper::drawLine(const ZLineSegment &line)
{
  getSlicePainter()->drawLine(getPainter(), line);
}

void ZSliceCanvasPaintHelper::drawPoint(double x, double y, double z)
{
  getSlicePainter()->drawPoint(getPainter(), x, y, z);
}

void ZSliceCanvasPaintHelper::drawPlanePolyline(
    QPainter *painter, const std::vector<QPointF> &points,
    double z, neutu::EAxis sliceAxis) const
{
  getSlicePainter()->drawPlanePolyline(painter, points, z, sliceAxis);
}

void ZSliceCanvasPaintHelper::setPen(const QPen &pen)
{
  getPainter()->setPen(pen);
}

void ZSliceCanvasPaintHelper::setBrush(const QBrush &brush)
{
  getPainter()->setBrush(brush);
}

bool ZSliceCanvasPaintHelper::getPaintedHint() const
{
  return getSlicePainter()->getPaintedHint();
}
