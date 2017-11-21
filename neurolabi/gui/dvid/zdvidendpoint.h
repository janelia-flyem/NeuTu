#ifndef ZDVIDENDPOINT_H
#define ZDVIDENDPOINT_H

#include <QString>
#include <QByteArray>

class ZJsonObject;

class ZDvidEndPoint
{
public:
  ZDvidEndPoint();

  static QString GetResultEndPoint(
      const QString &group, const QByteArray &data, bool head);
  static QString GetTaskEndPoint(
      const QString &group, const QByteArray &task, bool head);

  static QString GetResultKeyEndPoint(const QString &group, const QString &key);
  static QString GetTaskKeyEndPoint(const QString &group, const QString &key);

private:
  static QString GetResultEndPoint(const QString &group);
  static QString GetTaskEndPoint(const QString &group);
  static QString GetKeyEndPoint(const QString &start, const QString &key);
  static QString GetHashKey(const QByteArray &data, bool head);
};

#endif // ZDVIDENDPOINT_H
