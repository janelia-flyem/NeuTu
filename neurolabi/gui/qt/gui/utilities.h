#ifndef QT_GUI_UTILITIES_H
#define QT_GUI_UTILITIES_H

#include <functional>

#include <QKeySequence>
#include <QString>
#include <QStringList>
#include <QPainter>
#include <QPoint>
#include <QPushButton>

class QLayoutItem;

namespace neutu {

QString GetKeyString(int key, const Qt::KeyboardModifiers &modifier);
void SetHtmlIcon(QPushButton *button, const QString &text);
void DrawText(QPainter &painter, const QPoint &pos, const QStringList &text);
void DrawText(QPainter &painter, const QPoint &pos, const QString &text);
void DrawText(QPainter &painter, const QSize &windowSize, int cornerIndex,
              const QStringList &text);
void HideLayout(QLayout *layout, bool removing);
void ClearLayout(
    QLayout *layout, std::function<void(QLayoutItem*)> processChild);
void ClearLayout(QLayout *layout);

}

#endif // UTILITIES_H
