#ifndef ZDVIDPATH_H
#define ZDVIDPATH_H

#include <QString>
#include <QByteArray>


/*!
 * \brief The class of composing dvid path
 */
class ZDvidPath
{
public:
  ZDvidPath();

  static QString GetResultPath(
      const QString &group, const QByteArray &data, bool head);
  static QString GetTaskPath(
      const QString &group, const QByteArray &task, bool head);

  static QString GetResultKeyPath(const QString &group, const QString &key);
  static QString GetTaskKeyPath(const QString &group, const QString &key);

private:
  static QString GetResultPath(const QString &group);
  static QString GetTaskPath(const QString &group);
  static QString GetKeyPath(const QString &start, const QString &key);
  static QString GetHashKey(const QByteArray &data, bool head);
};

#endif // ZDVIDPATH_H
