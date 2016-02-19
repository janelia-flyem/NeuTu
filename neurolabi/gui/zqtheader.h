#ifndef ZQTHEADER_H
#define ZQTHEADER_H

#ifndef _QT_GUI_USED_

class ZQObject {
  ZQObject(ZQObject *parent = 0x0) {}
};
#define QObject ZObject

class ZQImage {};
#define QImage ZQImage

class ZQPainter{};
#define QPainter ZQPainter

class ZQPoint {
public:
  ZQPoint() {}
  ZQPoint(int, int) {}
};
#define QPoint ZQPoint

class ZQPointF {
public:
  ZQPointF() {}
  ZQPointF(double, double) {}
};
#define QPointF ZQPointF

class ZQPaintDevice{};
#define QPaintDevice ZQPaintDevice

class ZQString{};
#define QString ZQString

class ZQStringList{};
#define QStringList ZQStringList

class ZQList{};
#define QList ZQList

class ZQColor{};
#define QColor ZQColor

class ZQRectF{};
#define QRectF ZQRectF

class ZQRect{};
#define QRect ZQRect

typedef double Zqreal;
#define qreal Zqreal
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
#include <QDebug>
#include <QPen>
#endif

#endif // ZQTHEADER_H
