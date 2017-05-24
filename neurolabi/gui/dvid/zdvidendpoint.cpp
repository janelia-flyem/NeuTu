#include "zdvidendpoint.h"

#include <QCryptographicHash>

ZDvidEndPoint::ZDvidEndPoint()
{
}

QString ZDvidEndPoint::GetResultEndPoint()
{
  return "result";
}

QString ZDvidEndPoint::GetResultEndPoint(const QString &key)
{
  return GetResultEndPoint() + "/key/" + key;
}

QString ZDvidEndPoint::GetResultEndPoint(const QByteArray &data)
{
  return GetResultEndPoint(
      QString(QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex()));
}


