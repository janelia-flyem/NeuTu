#ifndef ZJSONOBJECTPARSER_H
#define ZJSONOBJECTPARSER_H

#include <string>
#include "zjsonparser.h"

class ZJsonObject;

class ZJsonObjectParser
{
public:
  ZJsonObjectParser();

  template<typename T>
  T getValue(const ZJsonObject &obj, const std::string &key) const;

  template<typename T>
  T getValue(const ZJsonObject &obj, const std::string &key,
             const T &defaultValue) const;

  template<typename T>
  T getValue(const ZJsonObject &obj, const char *key) const;

  template<typename T>
  T getValue(const ZJsonObject &obj, const char *key,
             const T &defaultValue) const;


//  template<typename T>
//  std::enable_if<std::is_integral<T>::value>::type
//  ZJsonParser::getValue(const ZJsonObject &obj, const std::string &key,
//                        const T &defaultValue) const;

  std::string getValue(const ZJsonObject &obj, const char *key,
                       const char *defaultValue);
  std::string getValue(const ZJsonObject &obj, const std::string &key,
                       const char *defaultValue);
};

template<typename T>
T ZJsonObjectParser::getValue(const ZJsonObject &obj, const std::string &key) const
{
  return getValue<T>(obj, key.c_str());
}

template<typename T>
T ZJsonObjectParser::getValue(const ZJsonObject &obj, const std::string &key,
           const T &defaultValue) const
{
  return getValue(obj, key.c_str(), defaultValue);
}

template<typename T>
T ZJsonObjectParser::getValue(
        const ZJsonObject &obj, const char *key,
        const T &defaultValue) const
{
  if (obj.hasKey(key)) {
    return ZJsonParser().getValue<T>(obj[key]);
  }

  return defaultValue;
}


//template<>
//std::string ZJsonObjectParser::getValue(
//        const ZJsonObject &obj, const std::string &key,
//        const std::string &defaultValue) const;

//template<>
//std::string ZJsonObjectParser::getValue(
//        const ZJsonObject &obj, const char *key,
//        const std::string &defaultValue) const;

//template<>
//int ZJsonObjectParser::getValue(
//        const ZJsonObject &obj, const std::string &key,
//        const int &defaultValue) const;

//template<>
//int ZJsonObjectParser::getValue(
//        const ZJsonObject &obj, const char *key,
//        const int &defaultValue) const;

#endif // ZJSONOBJECTPARSER_H
