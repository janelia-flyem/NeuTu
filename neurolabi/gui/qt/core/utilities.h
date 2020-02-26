#ifndef QT_CORE_UTILITIES_H
#define QT_CORE_UTILITIES_H

#include <QString>

namespace neutu {

QString NormalizeServerAddress(
    const QString &address, const QString &scheme = "http");

}

#endif // UTILITIES_H
