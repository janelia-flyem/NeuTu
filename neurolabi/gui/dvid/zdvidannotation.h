#ifndef ZDVIDANNOTATION_H
#define ZDVIDANNOTATION_H

#include <string>

class ZJsonObject;

class ZDvidAnnotation
{
public:
  ZDvidAnnotation();

public:
  static void AddProperty(ZJsonObject &json, const std::string &key,
                          const std::string &value);
  static void AddProperty(ZJsonObject &json, const std::string &key,
                          bool value);
};

#endif // ZDVIDANNOTATION_H
