#ifndef QT_GUI_UTILITIES_H
#define QT_GUI_UTILITIES_H

#include <vector>

#include <QKeySequence>
#include <QString>
#include <QStringList>
#include <QPainter>
#include <QPoint>
#include <QPointF>
#include <QPushButton>

namespace neutu {

QString GetKeyString(int key, const Qt::KeyboardModifiers &modifier);
void SetHtmlIcon(QPushButton *button, const QString &text);
void HideLayout(QLayout *layout, bool removing);

void DrawText(QPainter &painter, const QPoint &pos, const QStringList &text);
void DrawText(QPainter &painter, const QPoint &pos, const QString &text);
void DrawText(QPainter &painter, const QSize &windowSize, int cornerIndex,
              const QStringList &text);

struct PixelCentered {
  PixelCentered(bool v) { value = v; }
  operator bool() const {
    return value;
  }
  bool value = false;
};

void RevisePen(
    QPainter *painter, std::function<void(QPen &pen)> revise);
void ReviseBrush(QPainter *painter, std::function<void(QBrush &brush)> revise);

void SetPenColor(QPainter *painter, QColor color);
void ScalePenAlpha(QPainter *painter, double s);

void DrawPoint(
    QPainter &painter, double x, double y, const PixelCentered p = true);
void DrawPoints(
    QPainter &painter, const std::vector<QPointF> &pts,
    const PixelCentered p = true);
void DrawCircle(
    QPainter &painter, double cx, double cy, double r,
    const PixelCentered &p = true);
void DrawRect(
    QPainter &painter, double x0, double y0, double x1, double y1,
    const PixelCentered &p = true);
void DrawLine(
    QPainter &painter, double x0, double y0, double x1, double y1,
    const PixelCentered &p = true);
void DrawLines(
    QPainter &painter, const std::vector<QLineF> &lines,
    const PixelCentered &p = true);
void DrawLines(
    QPainter &painter, const std::vector<QLine> &lines,
    const PixelCentered &p = true);
void DrawPolyline(
    QPainter &painter, const std::vector<QPointF> &pts,
    const PixelCentered p = true);
void DrawStar(
    QPainter &painter, double cx, double cy, double r,
    const PixelCentered &p = true);

void MakeStar(
    const QPointF &center, double radius, QPointF *ptArray, double shapeFactor);
std::vector<QPointF> MakeStar(
    const QPointF &center, double radius, double shapeFactor = 0.25);

}

#endif // UTILITIES_H
