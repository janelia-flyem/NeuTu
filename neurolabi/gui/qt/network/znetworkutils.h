#ifndef ZNETWORKUTILS_H
#define ZNETWORKUTILS_H

#include <QString>

class ZJsonObject;

class ZNetworkUtils
{
public:
  ZNetworkUtils();

  static bool HasHead(const QString &url);
  static QByteArray Get(const QString &url);
  static QByteArray Post(const QString &url, const QByteArray &payload);

  static ZJsonObject ReadJsonObjectMemo(const std::string& url);
};

#endif // ZNETWORKUTILS_H
