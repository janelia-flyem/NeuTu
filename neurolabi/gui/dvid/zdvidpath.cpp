#include "zdvidpath.h"

#include <QCryptographicHash>
#include "zjsonobject.h"
#include "zdviddata.h"

ZDvidPath::ZDvidPath()
{

}

QString ZDvidPath::GetResultPath(const QString &group)
{
  return ZDvidData::GetResultName<QString>(group.toStdString());
}

QString ZDvidPath::GetTaskPath(const QString &group)
{
  return ZDvidData::GetTaskName<QString>(group.toStdString());
}

QString ZDvidPath::GetHashKey(const QByteArray &data, bool head)
{
  QString key(QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex());
  if (head) {
    key = "head__" + key;
  }

  return key;
}

QString ZDvidPath::GetKeyPath(const QString &start, const QString &key)
{
  return start + "/key/" + key;
}

QString ZDvidPath::GetResultKeyPath(
    const QString &group, const QString &key)
{
  return GetKeyPath(GetResultPath(group), key);
}

QString ZDvidPath::GetResultPath(
    const QString &group, const QByteArray &data, bool head)
{
  return GetResultKeyPath(group, GetHashKey(data, head));
}

QString ZDvidPath::GetTaskKeyPath(const QString &group, const QString &key)
{
  return GetKeyPath(GetTaskPath(group), key);
}

QString ZDvidPath::GetTaskPath(
    const QString &group, const QByteArray &data, bool head)
{
  return GetTaskKeyPath(group, GetHashKey(data, head));
}
