#include "zpixmap.h"

#include <QPainter>

ZPixmap::ZPixmap() : m_isVisible(false)
{
}

ZPixmap::ZPixmap(const QSize &size) : QPixmap(size), m_isVisible(false)
{
  fill(Qt::transparent);
}

ZPixmap::ZPixmap(int width, int height) :
  QPixmap(width, height), m_isVisible(false)
{
  fill(Qt::transparent);
}

const ZStTransform& ZPixmap::getTransform() const
{
  return m_transform;
}

void ZPixmap::setScale(double sx, double sy)
{
  m_transform.setScale(sx, sy);
}

void ZPixmap::setOffset(double dx, double dy)
{
  m_transform.setOffset(dx, dy);
}

void ZPixmap::cleanUp()
{
#if 0
  if (m_cleanBuffer.isNull()) {
    m_cleanBuffer = QPixmap(width(), height());
    m_cleanBuffer.fill(Qt::transparent);
  }
#endif

#if 0
  if (m_cleanMask.isNull()) {
    m_cleanMask = QBitmap(width(), height());
    m_cleanMask.clear();
  }

  setMask(m_cleanMask);
#endif

#if 0
  QPainter painter;
  painter.begin(this);
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.fillRect(QRect(0, 0, width(), height()), QColor(0, 0, 0, 0));

//  painter.setCompositionMode(QPainter::CompositionMode_Source);
//  painter.drawPixmap(0, 0, m_cleanBuffer);
  painter.end();
#endif

  clean(QRect(0, 0, width(), height()));

//  fill(Qt::transparent);
  m_isVisible = false;
}

void ZPixmap::clean(const QRect &rect)
{
  QPainter painter;
  painter.begin(this);
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.fillRect(rect, QColor(0, 0, 0, 0));
  painter.end();

  /*
  QPainter painter;
  painter.begin(this);
  QPixmap pixmap(rect.width(), rect.height());
  pixmap.fill(Qt::transparent);
  painter.drawPixmap(rect.left(), rect.top(), pixmap);
  painter.end();
  */
}

QRectF ZPixmap::getActiveArea(NeuTube::ECoordinateSystem coord) const
{
  switch (coord) {
  case NeuTube::COORD_WORLD:
    if (m_activeArea.isEmpty()) {
      return m_transform.getInverseTransform().transform(
            QRectF(0, 0, width(), height()));
    } else {
      return m_activeArea;
    }
  case NeuTube::COORD_CANVAS:
    if (m_activeArea.isEmpty()) {
      return QRectF(0, 0, width(), height());
    } else {
      return m_transform.transform(m_activeArea);
    }
  default:
    break;
  }

  return QRectF();
}

bool ZPixmap::isFullyActive() const
{
  if (m_activeArea.isEmpty()) {
    return true;
  }
  return m_transform.transform(m_activeArea).contains(
        QRectF(0, 0, width(), height()));
}
