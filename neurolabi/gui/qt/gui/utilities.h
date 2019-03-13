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
void DrawText(QPainter &painter, const QPoint &pos, const QStringList &text);
void DrawText(QPainter &painter, const QPoint &pos, const QString &text);
void DrawText(QPainter &painter, const QSize &windowSize, int cornerIndex,
              const QStringList &text);
}

#endif // UTILITIES_H
