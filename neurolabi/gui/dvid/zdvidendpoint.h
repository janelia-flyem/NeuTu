#ifndef ZDVIDENDPOINT_H
#define ZDVIDENDPOINT_H

#include <QString>
#include <QByteArray>

class ZDvidEndPoint
{
public:
  ZDvidEndPoint();

  static QString GetResultEndPoint();
  static QString GetResultEndPoint(const QString &key);
  static QString GetResultEndPoint(const QByteArray &data);
};

#endif // ZDVIDENDPOINT_H
