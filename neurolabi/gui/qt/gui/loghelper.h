#ifndef LOGHELPER_H
#define LOGHELPER_H

#include <QString>

class QMouseEvent;
class QWheelEvent;
class QKeyEvent;

namespace neutu {

void LogMouseEvent(QMouseEvent *event, const QString &action, const QString &window);
void LogMouseEvent(QWheelEvent *event, const QString &window);
void LogKeyEvent(QKeyEvent *event, const QString &window);
}


#endif // LOGHELPER_H
