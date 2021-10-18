#include "zrect2d.h"

#include <cmath>
#include <QRect>
#include <QRectF>
#include <QPen>

#include "common/math.h"
#include "common/utilities.h"

#include "geometry/zcuboid.h"
#include "geometry/zintcuboid.h"
#include "geometry/zgeometry.h"
#include "geometry/zlinesegment.h"
#include "geometry/zaffinerect.h"

#include "data3d/displayconfig.h"
#include "vis2d/zslicepainter.h"
#include "vis2d/zsttransform.h"

#include "qt/gui/utilities.h"

ZRect2d::ZRect2d()
{
  init(0, 0, 0, 0);
}

ZRect2d::ZRect2d(int x0, int y0, int width, int height)
{
  init(x0, y0, width, height);
}

ZRect2d::~ZRect2d()
{
#ifdef _DEBUG_
  std::cout << "Destroying ZRect2d: " << getSource() << std::endl;
#endif
}

void ZRect2d::init(int x0, int y0, int width, int height)
{
  set(x0, y0, width, height);
  m_z = 0;
  m_isPenetrating = false;
  m_zSpan = 0;
  m_type = GetType();
  m_coordSpace = neutu::data3d::ESpace::CANVAS;
  useCosmeticPen(true);
}

void ZRect2d::set(int x0, int y0, int width, int height)
{
  m_x0 = x0;
  m_y0 = y0;
  m_width = width;
  m_height = height;
}

ZCuboid ZRect2d::getBoundBox() const
{
  if (_getBoundBox) {
    return _getBoundBox(*this);
  } else {
    return ZCuboid::FromIntCuboid(getIntBoundBox());
  }
}

ZIntCuboid ZRect2d::getIntBoundBox() const
{
  if (_getBoundBox) {
    return _getBoundBox(*this).toIntCuboid();
  } else {
    return ZIntCuboid(getMinX(), getMinY(), getZ() - m_zSpan,
                      getMaxX(), getMaxY(), getZ() + m_zSpan);
  }
}

ZAffineRect ZRect2d::getAffineRect() const
{
  if (_getAffineRect) {
    return _getAffineRect(*this);
  } else {
    ZAffineRect rect;
    rect.setPlane(zgeom::GetPlane(getSliceAxis()));
    rect.setSize(getWidth(), getHeight());
    rect.setCenter(getCenter().toPoint());

    return rect;
  }
}

bool ZRect2d::isValid() const
{
  return m_width > 0 && m_height > 0;
}

bool ZRect2d::isSliceVisible(int z, neutu::EAxis /*sliceAxis*/) const
{
  bool visible = isValid();

  if (visible) {
    if (!m_isPenetrating) {
      visible = abs(z - m_z) <= m_zSpan;
    }
  }


  return visible;
}

void ZRect2d::preparePen(QPen &pen) const
{
  pen.setColor(m_color);
  pen.setWidthF(getPenWidth());
//  if (isSelected()) {
//    pen.setWidth(pen.width() + 5);
//    pen.setStyle(Qt::DashLine);
//  }

  pen.setCosmetic(m_usingCosmeticPen);
}

bool ZRect2d::isEmpty() const
{
  return getWidth() <= 0 || getHeight() <= 0;
}

bool ZRect2d::display(QPainter *painter, const DisplayConfig &config) const
{
  if (isVisible() && !isEmpty() && m_viewId == config.getViewId()) {
    neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});
    QPen pen(getColor());
    pen.setCosmetic(m_usingCosmeticPen);
    painter->setPen(pen);

    int x1 = m_x0 + m_width - 1;
    int y1 = m_y0 + m_height - 1;
    neutu::DrawIntRect(*painter, m_x0, m_y0, x1, y1);

    if (isSelected()) {
      neutu::DrawLine(*painter, QPoint(m_x0, m_y0), QPoint(x1, y1));
    }

    this->_hit = [=](const ZStackObject *obj, double x, double y, double z) {
      auto s = dynamic_cast<const ZRect2d*>(obj);
      ZPoint pt = config.getTransform().transform(x, y, z);
      return ((pt.getX() >= s->m_x0 - 5 && pt.getY() >= s->m_y0 - 5 &&
               pt.getX() < s->m_x0 + s->m_width + 5 && pt.getY() < s->m_y0 + s->m_height + 5) &&
              !(pt.getX() >= s->m_x0 + 5 && pt.getY() >= s->m_y0 + 5 &&
                pt.getX() < s->m_x0 + s->m_width - 5 && pt.getY() < s->m_y0 + s->m_height - 5));
    };

    this->_getBoundBox = [=](const ZRect2d &rect) {
      ZPoint minCorner = config.getTransform().inverseTransform(
            rect.m_x0, rect.m_y0, rect.m_z - rect.m_zSpan);
      ZPoint maxCorner = config.getTransform().inverseTransform(
            rect.m_x0 + rect.m_width,
            rect.m_y0 + rect.m_height, rect.m_z + rect.m_zSpan);
      return ZCuboid(minCorner, maxCorner);
    };

    this->_getAffineRect = [=](const ZRect2d &rect) {
      ZAffineRect ar;
      ar.setPlane(config.getTransform().getCutPlane());
      ZIntPoint pt = rect.getCenter();
      ar.setSize(rect.getWidth() / config.getTransform().getScale(),
                 rect.getHeight() / config.getTransform().getScale());
      ar.setCenter(config.getTransform().inverseTransform(pt.getX(), pt.getY()));
      return ar;
    };

    return true;
  } else {
    this->_hit = [](const ZStackObject*, double,double,double) { return false; };
  }
#if 0
  if (isVisible() && (getSliceAxis() == config.getSliceAxis())) {
    ZSlice3dPainter s3Painter;
    s3Painter.setModelViewTransform(config.getWorldViewTransform());
    s3Painter.setViewCanvasTransform(config.getViewCanvasTransform());
    neutu::ApplyOnce ao([&]() {painter->save();}, [&]() {painter->restore();});

    QPen pen(getColor());
    pen.setCosmetic(m_usingCosmeticPen);

    double x0 = m_x0;
    double y0 = m_y0;
    double x1 = x0 + m_width;
    double y1 = y0 + m_height;
    double z = m_z;

    ZPoint center = config.getCutPlane().getOffset();
    center.shiftSliceAxisInverse(getSliceAxis());

    double dz = z - center.getZ();

    if (!m_isPenetrating && std::fabs(dz) > std::max(double(m_zSpan), 0.5)) {
      pen.setStyle(Qt::DashLine);
    }

    painter->setPen(pen);

    ZSlice2dPainter s2Painter;
    s2Painter.setViewPlaneTransform(config.getViewCanvasTransform());

    x0 -= center.getX();
    y0 -= center.getY();
    x1 -= center.getX();
    y1 -= center.getY();

    s2Painter.drawRect(painter, x0, y0, x1, y1);

    if (isSelected()) {
      s2Painter.drawLine(painter, x0, y0, x1, y1);
      s2Painter.drawLine(painter, x0, y1, x1, y0);
    }

    return s2Painter.getPaintedHint();
  }
#endif

  return false;
}

#if 0
void ZRect2d::display(ZPainter &painter, int slice, EDisplayStyle /*option*/,
                      neutu::EAxis sliceAxis) const
{
  if (sliceAxis != m_sliceAxis) {
    return;
  }

  int z = slice + painter.getZOffset();
  if (!(isSliceVisible(z, sliceAxis) || (slice < 0))) {
    return;
  }

  QPen pen;
  preparePen(pen);

  painter.setPen(pen);
  painter.setBrush(Qt::NoBrush);

  painter.drawRect(m_x0, m_y0, m_width, m_height);

  if (isSelected()) {
    QColor color = getColor();
    color.setAlpha(128);
    pen.setColor(color);
    painter.setPen(pen);
    painter.drawLine(getMinX(), getMinY(), getMaxX(), getMaxY());
    painter.drawLine(getMinX(), getMaxY(), getMaxX(), getMinY());
  }
}

bool ZRect2d::display(QPainter *rawPainter, int /*z*/, EDisplayStyle /*option*/,
             EDisplaySliceMode /*sliceMode*/, neutu::EAxis sliceAxis) const
{
  if (sliceAxis != m_sliceAxis) {
    return false;
  }

  bool painted = false;

  if (rawPainter == NULL || !isVisible()) {
    return painted;
  }

  QPen pen;
  preparePen(pen);

  rawPainter->setPen(pen);
  rawPainter->setBrush(Qt::NoBrush);


  int width = m_width;
  int height = m_height;
  int x0 = m_x0;
  int y0 = m_y0;

  if (width < 0) {
    width = -width;
    x0 -= width - 1;
  }

  if (height < 0) {
    height = -height;
    y0 -= height - 1;
  }

//  rawPainter->drawEllipse(x0, y0, width/2, height/2);
  rawPainter->drawRect(x0, y0, width, height);

  return true;
}
#endif

void ZRect2d::setMaxCorner(int x, int y)
{
  m_width = x - m_x0 + 1;
  m_height = y - m_y0 + 1;
}

void ZRect2d::setMinCorner(int x, int y)
{
  m_x0 = x;
  m_y0 = y;
}

void ZRect2d::setStartCorner(int x, int y)
{
  m_sx0 = x;
  m_sy0 = y;

  set(x, y, 0, 0);
}

void ZRect2d::setEndCorner(int x, int y)
{
  setMinCorner(std::min(m_sx0, x), std::min(m_sy0, y));
  setMaxCorner(std::max(m_sx0, x), std::max(m_sy0, y));
}

void ZRect2d::setSize(int width, int height)
{
  m_width = width;
  m_height = height;
}

bool ZRect2d::makeValid()
{
  if (m_width == 0 || m_height == 0) {
    return false;
  }

  if (m_width < 0) {
    m_width = -m_width;
    m_x0 -= m_width - 1;
  }

  if (m_height < 0) {
    m_height = -m_height;
    m_y0 -= m_height - 1;
  }

  return true;
}

void ZRect2d::updateZSpanWithRadius()
{
  m_zSpan = neutu::iround(
        std::sqrt(m_width * m_width + m_height * m_height) * 0.5);
}

void ZRect2d::updateZSpanWithMinSide()
{
  m_zSpan = std::min(m_width / 2, m_height / 2);
}

int ZRect2d::getMinX() const
{
  return m_x0;
}

int ZRect2d::getMinY() const
{
  return m_y0;
}

int ZRect2d::getMaxX() const
{
  return m_width + m_x0 - 1;
}

int ZRect2d::getMaxY() const
{
  return m_height + m_y0 - 1;
}

bool ZRect2d::contains(double x, double y) const
{
  return ((x >= m_x0 && y >= m_y0 &&
       x < m_x0 + m_width && y < m_y0 + m_height));
}

ZIntPoint ZRect2d::getCenter() const
{
  return ZIntPoint(m_x0 + m_width / 2, m_y0 + m_height / 2, m_z);
}

void ZRect2d::setViewId(int viewId)
{
  m_viewId = viewId;
}

/*
bool ZRect2d::hit(double x, double y, neutu::EAxis axis)
{
  if (m_sliceAxis != axis) {
    return false;
  }

  return ((x >= m_x0 - 5 && y >= m_y0 - 5 &&
       x < m_x0 + m_width + 5 && y < m_y0 + m_height + 5) &&
      !(x >= m_x0 + 5 && y >= m_y0 + 5 &&
        x < m_x0 + m_width - 5 && y < m_y0 + m_height - 5));
}

bool ZRect2d::hit(double x, double y, double z)
{
  double wx = x;
  double wy = y;
  double wz = z;

  zgeom::ShiftSliceAxis(wx, wy, wz, m_sliceAxis);

  if (m_isPenetrating) {
    return hit(wx, wy, m_sliceAxis);
  }

  if (neutu::iround(wz) == m_z) {
    return ((wx >= m_x0 - 5 && wy >= m_y0 - 5 &&
             wx < m_x0 + m_width + 5 && wy < m_y0 + m_height + 5) &&
            !(wx >= m_x0 + 5 && wy >= m_y0 + 5 &&
              wx < m_x0 + m_width - 5 && wy < m_y0 + m_height - 5));
  }

  return false;
}
*/

bool ZRect2d::hit(double x, double y, double z, int viewId)
{
  if (m_viewId == viewId) {
    return ZStackObject::hit(x, y, z);
  }

  return false;
}

bool ZRect2d::IsEqual(const QRect &rect1, const QRect &rect2)
{
  return (rect1.left() == rect2.left()) && (rect1.top() == rect2.top()) &&
      (rect1.right() == rect2.right()) && (rect1.top() == rect2.top());
}

bool ZRect2d::IsEqual(const QRectF &rect1, const QRectF &rect2)
{
  return (rect1.left() == rect2.left()) && (rect1.top() == rect2.top()) &&
      (rect1.right() == rect2.right()) && (rect1.top() == rect2.top());
}

QRect ZRect2d::QRectBound(const QRectF &rect)
{
  QRect out;
  out.setLeft(std::floor(rect.left()));
  out.setTop(std::floor(rect.top()));
  out.setBottom(std::ceil(rect.bottom()));
  out.setRight(std::ceil(rect.right()));

  return out;
}

QRectF ZRect2d::CropRect(
    const QRectF &sourceRectIn, const QRectF &sourceRectOut,
    const QRectF &targetRectIn)
{
  if (IsEqual(sourceRectIn, sourceRectOut)) {
    return targetRectIn;
  }

  QRectF out;
  ZStTransform transform;
  transform.estimate(sourceRectIn, sourceRectOut);
  out = transform.transform(targetRectIn);

  return out;
}


//ZSTACKOBJECT_DEFINE_CLASS_NAME(ZRect2d)
