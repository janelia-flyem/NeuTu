#include "utilities.h"

#include <QUrl>
#include <QDebug>

QString neutu::NormalizeServerAddress(
    const QString &address, const QString &scheme)
{
  QString newAddress = address.trimmed();
  if (!newAddress.contains("://")) {
    newAddress = scheme + "://" + address;
  }

  QUrl url(newAddress);
  url.setPath("");

  QString result =url.toString();
  if (result.endsWith("/")) {
    result.remove(result.size() - 1, 1);
  }

  return result;
}
