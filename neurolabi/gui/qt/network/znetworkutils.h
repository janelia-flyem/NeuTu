#ifndef ZNETWORKUTILS_H
#define ZNETWORKUTILS_H

#include <QString>

#include "znetworkdefs.h"

class ZJsonObject;
class QNetworkRequest;

class ZNetworkUtils
{
public:
  ZNetworkUtils();

  static bool HasHead(const QString &url, int timeout = 2000);
  static QByteArray Get(const QString &url);
  static QByteArray Post(const QString &url, const QByteArray &payload);

  static ZJsonObject ReadJsonObjectMemo(const std::string& url);

  static bool IsAvailable(
      const QNetworkRequest &request, const QByteArray &method, int timeout);
  static bool IsAvailable(
      const QString &url, const QByteArray &method, int timeout = 2000);
  static bool IsAvailable(
      const QString &url, znetwork::EOperation op, int timeout = 2000);
};

#endif // ZNETWORKUTILS_H
