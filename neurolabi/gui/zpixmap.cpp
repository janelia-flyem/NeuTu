#include "zpixmap.h"

#include <iostream>

#include <QPainter>
#include <QtConcurrentRun>
#include <QApplication>

ZPixmap::ZPixmap() : m_isVisible(false)
{
  m_cleanWatcher = new QFutureWatcher<void>(NULL);
}

ZPixmap::ZPixmap(const QSize &size) : QPixmap(size), m_isVisible(false)
{
  fill(Qt::transparent);
  m_cleanWatcher = new QFutureWatcher<void>(NULL);
}

ZPixmap::ZPixmap(int width, int height) :
  QPixmap(width, height), m_isVisible(false)
{
  fill(Qt::transparent);
  m_cleanWatcher = new QFutureWatcher<void>(NULL);
}

ZPixmap::~ZPixmap()
{
  if (m_cleanWatcher->isRunning()) {
    m_cleanWatcher->waitForFinished();
  }
  delete m_cleanWatcher;
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

void ZPixmap::cleanFunc(QPixmap *pixmap)
{
//  std::cout << "background cleaning" << std::endl;
  pixmap->fill(Qt::transparent);
//  std::cout << "done" << std::endl;
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

#if QT_VERSION >= QT_VERSION_CHECK(4, 8, 0)
  if (width() * height() <= 4096 * 4096) {
    fill(Qt::transparent);
  } else {
    if (m_cleanBuffer.isNull()) {
      m_cleanBuffer = QPixmap(width(), height());
      m_cleanBuffer.fill(Qt::transparent);
    } else {
      if (!m_cleanWatcher->isFinished()) {
//        std::cout << "Waiting..." << std::endl;
        m_cleanWatcher->waitForFinished();
      }
    }

    swap(m_cleanBuffer);

    QFuture<void> future = QtConcurrent::run(this, &ZPixmap::cleanFunc, &m_cleanBuffer);
    m_cleanWatcher->setFuture(future);
  }
#else
  fill(Qt::transparent);
#endif
//  m_cleanFuture = future;

  //clean(QRect(0, 0, width(), height()));

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
