#ifndef ZNEUROGLANCERPATHPARSER_H
#define ZNEUROGLANCERPATHPARSER_H

#include <QString>

class ZJsonObject;
class ZDvidEnv;

class ZNeuroglancerPathParser
{
public:
  ZNeuroglancerPathParser();

  static ZJsonObject ParseDataSpec(const QString &url);
  static ZDvidEnv MakeDvidEnvFromUrl(const QString &url);

};

#endif // ZNEUROGLANCERPATHPARSER_H
