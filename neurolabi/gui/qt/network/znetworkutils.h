#ifndef ZNETWORKUTILS_H
#define ZNETWORKUTILS_H

#include <QString>

class ZNetworkUtils
{
public:
  ZNetworkUtils();

  static bool HasHead(const QString &url);
  static QByteArray Get(const QString &url);
  static QByteArray Post(const QString &url, const QByteArray &payload);
};

#endif // ZNETWORKUTILS_H
