#include "zpixmap.h"

#include <iostream>

#include <QPainter>
#include <QtConcurrentRun>
#include <QApplication>
#include <QGraphicsBlurEffect>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

#include "zstack.hxx"

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
  return m_objTransform;
}

const ZStTransform& ZPixmap::getProjTransform() const
{
  return m_projTransform;
}

ZStTransform& ZPixmap::getProjTransform()
{
  return m_projTransform;
}

void ZPixmap::updateProjTransform(
    const QRect &viewPort, const QRectF &newProjRegion)
{
  m_projTransform.estimate(m_objTransform.transform(viewPort), newProjRegion);
}

void ZPixmap::setTransform(const ZStTransform &transform)
{
  m_objTransform = transform;
}

void ZPixmap::setScale(double sx, double sy)
{
  m_objTransform.setScale(sx, sy);
}

void ZPixmap::setOffset(double dx, double dy)
{
  m_objTransform.setOffset(dx, dy);
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

void ZPixmap::matchProj()
{
  m_projTransform = getTransform().getInverseTransform();
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

QRectF ZPixmap::getActiveArea(neutu::ECoordinateSystem coord) const
{
  switch (coord) {
  case neutu::ECoordinateSystem::WORLD_2D:
    if (m_activeArea.isEmpty()) {
      return m_objTransform.getInverseTransform().transform(
            QRectF(0, 0, width(), height()));
    } else {
      return m_activeArea;
    }
  case neutu::ECoordinateSystem::CANVAS:
    if (m_activeArea.isEmpty()) {
      return QRectF(0, 0, width(), height());
    } else {
      return m_objTransform.transform(m_activeArea);
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
  return m_objTransform.transform(m_activeArea).contains(
        QRectF(0, 0, width(), height()));
}

ZStack* ZPixmap::toPlainStack(neutu::EColor color, uint8_t maskValue)
{
  ZStack *stack = new ZStack(GREY, width(), height(), 1, 1);
  size_t offset = 0;
  uint8_t *array = stack->array8();
  QImage image = toImage();
  for (int y = 0; y < height(); ++y) {
    for (int x = 0; x < width(); ++x) {
      QRgb rgb = image.pixel(x, y);
      bool isForeground = false;
      switch (color) {
      case neutu::EColor::RED:
        if ((qRed(rgb) > qGreen(rgb)) && (qRed(rgb) > qBlue(rgb))) {
          isForeground = true;
        }
        break;
      case neutu::EColor::GREEN:
        if ((qGreen(rgb) > qRed(rgb)) && (qGreen(rgb) > qBlue(rgb))) {
          isForeground = true;
        }
        break;
      case neutu::EColor::BLUE:
        if ((qBlue(rgb) > qRed(rgb)) && (qBlue(rgb) > qGreen(rgb))) {
          isForeground = true;
        }
        break;
      default:
        break;
      }

      if (isForeground) {
        array[offset] = maskValue;
      } else {
        array[offset] = 0;
      }
      ++offset;
    }
  }

  return stack;
}

ZStack* ZPixmap::toPlainStack(uint8_t maskValue)
{
  ZStack *stack = NULL;

  QImage image = toImage();

  stack = new ZStack(GREY, width(), height(), 1, 1);
  size_t offset = 0;
  uint8_t *array = stack->array8();
  for (int y = 0; y < height(); ++y) {
    for (int x = 0; x < width(); ++x) {
      QRgb rgb = image.pixel(x, y);
      if (qRed(rgb) > 0 || qGreen(rgb) > 0 || qBlue(rgb) > 0) {
        array[offset] = maskValue;
      } else {
        array[offset] = 0;
      }
      ++offset;
    }
  }

  return stack;
}
