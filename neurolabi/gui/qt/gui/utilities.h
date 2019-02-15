#ifndef QT_GUI_UTILITIES_H
#define QT_GUI_UTILITIES_H

#include <QKeySequence>
#include <QString>

namespace neutu {

QString GetKeyString(int key, const Qt::KeyboardModifiers &modifier);

}

#endif // UTILITIES_H
