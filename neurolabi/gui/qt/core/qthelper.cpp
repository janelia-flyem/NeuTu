#include "qthelper.h"

#include "zjsonobject.h"

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

QJsonObject neutu::ToQJsonObject(const ZJsonObject &obj)
{
  QJsonObject qobj;

  if (!obj.isEmpty()) {
    QJsonDocument jdoc = QJsonDocument::fromJson(
          QString::fromStdString(obj.dumpString(0)).toUtf8());
    qobj = jdoc.object();
  }

  return qobj;
}

QJsonValue neutu::ToQJsonValue(const ZJsonObject &obj)
{
  return QJsonValue(ToQJsonObject(obj));
}
