#ifndef ZNETWORKUTILS_H
#define ZNETWORKUTILS_H

#include <QString>

class ZNetworkUtils
{
public:
  ZNetworkUtils();

  static bool HasHead(const QString &url);
};

#endif // ZNETWORKUTILS_H
