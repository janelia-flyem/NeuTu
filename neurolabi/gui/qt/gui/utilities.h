#ifndef UTILITIES_H
#define UTILITIES_H

#include <QKeySequence>
#include <QString>

namespace neutu {

QString GetKeyString(int key, const Qt::KeyboardModifiers &modifier);

}

#endif // UTILITIES_H
