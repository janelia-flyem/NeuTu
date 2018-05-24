#ifndef QTHELPER_H
#define QTHELPER_H

#include <QtCore>

namespace neutube {
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
}
#endif // QTHELPER_H
