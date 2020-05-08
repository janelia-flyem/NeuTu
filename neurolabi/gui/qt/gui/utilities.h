#ifndef QT_GUI_UTILITIES_H
#define QT_GUI_UTILITIES_H

#include <QKeySequence>
#include <QString>
#include <QStringList>
#include <QPainter>
#include <QPoint>
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
void DrawCircle(
    QPainter &painter, double cx, double cy, double r,
    const PixelCentered p = true);
void DrawRect(
    QPainter &painter, double x0, double y0, double x1, double y1,
    const PixelCentered p = true);
void DrawLine(
    QPainter &painter, double x0, double y0, double x1, double y1,
    const PixelCentered p = true);


}

#endif // UTILITIES_H
