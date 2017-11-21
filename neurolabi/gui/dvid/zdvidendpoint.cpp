#include "zdvidendpoint.h"

#include <QCryptographicHash>
#include "zjsonobject.h"

ZDvidEndPoint::ZDvidEndPoint()
{
}

QString ZDvidEndPoint::GetResultEndPoint(const QString &group)
{
  return "result_" + group;
}

QString ZDvidEndPoint::GetTaskEndPoint(const QString &group)
{
  return "task_" + group;
}

QString ZDvidEndPoint::GetHashKey(const QByteArray &data, bool head)
{
  QString key(QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex());
  if (head) {
    key = "head__" + key;
  }

  return key;
}

QString ZDvidEndPoint::GetKeyEndPoint(const QString &start, const QString &key)
{
  return start + "/key/" + key;
}

QString ZDvidEndPoint::GetResultKeyEndPoint(
    const QString &group, const QString &key)
{
  return GetKeyEndPoint(GetResultEndPoint(group), key);
}

QString ZDvidEndPoint::GetResultEndPoint(
    const QString &group, const QByteArray &data, bool head)
{
  return GetResultKeyEndPoint(group, GetHashKey(data, head));
}

QString ZDvidEndPoint::GetTaskKeyEndPoint(const QString &group, const QString &key)
{
  return GetKeyEndPoint(GetTaskEndPoint(group), key);
}

QString ZDvidEndPoint::GetTaskEndPoint(
    const QString &group, const QByteArray &data, bool head)
{
  return GetTaskKeyEndPoint(group, GetHashKey(data, head));
}
