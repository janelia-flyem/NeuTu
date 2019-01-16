#include "qthelper.h"

bool neutu::ConnectFunc(const QObject* obj1, const char *signal,
                          const QObject *obj2, const char *slot,
                          Qt::ConnectionType connetionType)
{
  return QObject::connect(obj1, signal, obj2, slot, connetionType);
}

bool neutu::DisconnectFunc(const QObject* obj1, const char *signal,
                             const QObject *obj2, const char *slot,
                             Qt::ConnectionType /*connetionType*/)
{
  return QObject::disconnect(obj1, signal, obj2, slot);
}
