#ifndef ZQTHEADER_H
#define ZQTHEADER_H

#ifndef _QT_GUI_USED_
class QObject {
  QObject(QObject *parent = 0x0) {}
};
class QImage {};
class QPainter{};
class QPoint {
public:
  QPoint() {}
  QPoint(int, int) {}
};
class QPointF {
public:
  QPointF() {}
  QPointF(double, double) {}
};
class QPaintDevice{};
class QString{};
class QStringList{};
class QList{};
class QColor{};
class QRectF{};
class QRect{};
typedef double qreal;
#else
#include <QObject>
#include <QColor>
#include <QPainter>
#include <QImage>
#include <QPoint>
#include <QPointF>
#include <QPaintDevice>
#include <QString>
#include <QList>
#include <QRectF>
#include <QRect>
#endif

#endif // ZQTHEADER_H
