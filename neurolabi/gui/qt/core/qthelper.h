#ifndef QTHELPER_H
#define QTHELPER_H

#include <QtCore>

class ZJsonObject;

namespace neutu {
typedef bool FConnectAction(
    const QObject*, const char *,
    const QObject *, const char *,
    Qt::ConnectionType connetionType);

bool ConnectFunc(const QObject* obj1, const char *signal,
                 const QObject *obj2, const char *slot,
                 Qt::ConnectionType connetionType);
bool DisconnectFunc(const QObject* obj1, const char *signal,
                    const QObject *obj2, const char *slot,
                    Qt::ConnectionType connetionType);

QJsonObject ToQJsonObject(const ZJsonObject &obj);
QJsonValue ToQJsonValue(const ZJsonObject &obj);
}

#endif // QTHELPER_H
