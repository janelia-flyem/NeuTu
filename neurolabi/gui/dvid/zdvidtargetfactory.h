#ifndef ZDVIDTARGETFACTORY_H
#define ZDVIDTARGETFACTORY_H

#include <string>
#include <QString>

class ZDvidTarget;
class ZJsonObject;

class ZDvidTargetFactory
{
public:
  ZDvidTargetFactory();

  static ZDvidTarget MakeFromSourceString(const std::string &sourceString);
  static ZDvidTarget MakeFromSourceString(const QString &sourceString);
  static ZDvidTarget MakeFromSourceString(const char *sourceString);

  static ZDvidTarget MakeFromUrlSpec(const std::string &urlSpec);
  static ZDvidTarget MakeFromUrlSpec(const QString &urlSpec);
  static ZDvidTarget MakeFromUrlSpec(const char *urlSpec);


  static ZDvidTarget MakeFromSpec(const std::string &spec);
  static ZDvidTarget MakeFromSpec(const QString &spec);
  static ZDvidTarget MakeFromSpec(const char *spec);

  static ZDvidTarget Make(const ZJsonObject &obj);

  static ZDvidTarget MakeFromJsonSpec(const std::string &spec);

};

#endif // ZDVIDTARGETFACTORY_H
