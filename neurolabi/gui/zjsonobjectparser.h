#ifndef ZJSONOBJECTPARSER_H
#define ZJSONOBJECTPARSER_H

#include <vector>
#include <string>
#include "zjsonparser.h"

class ZJsonObject;

class ZJsonObjectParser
{
public:
  ZJsonObjectParser();
  ZJsonObjectParser(const ZJsonObject &obj);

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

  template<typename T>
  T getValue(const std::string &key, const T &defaultValue) const;
  template<typename T>
  T getValue(const char *key, const T &defaultValue) const;

  /*!
   * \brief Get value from a list of candidate keys
   *
   * The first existing key in \a cadidateKeys will be retrieved first even
   * if the value is not valid.
   */
  template<typename T>
  T getValue(
      const ZJsonObject &obj, const std::vector<std::string> &candidateKeys,
      const T &defaultValue);

  std::string getValue(
      const ZJsonObject &obj, const std::vector<std::string> &candidateKeys,
      const char* defaultValue);
//  std::string getValue(
//      const ZJsonObject &obj, const std::vector<std::string> &candidateKeys,
//      const std::string &defaultValue);

  std::string getValue(const ZJsonObject &obj, const char *key,
                       const char *defaultValue) const;
  std::string getValue(const ZJsonObject &obj, const std::string &key,
                       const char *defaultValue) const;

  std::string getValue(const char *key, const char *defaultValue) const;
  std::string getValue(const std::string &key, const char *defaultValue) const;

private:
  ZJsonObject m_obj;
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

template<typename T>
T ZJsonObjectParser::getValue(
    const ZJsonObject &obj, const std::vector<std::string> &candidateKeys,
    const T &defaultValue)
{
  for (const std::string &key : candidateKeys) {
    if (obj.hasKey(key)) {
      return getValue(obj, key, defaultValue);
    }
  }

  return defaultValue;
}

template<typename T>
T ZJsonObjectParser::getValue(
    const std::string &key, const T &defaultValue) const
{
  return getValue(m_obj, key, defaultValue);
}

template<typename T>
T ZJsonObjectParser::getValue(const char *key, const T &defaultValue) const
{
  return getValue(m_obj, key, defaultValue);
}

//template<>
//std::string ZJsonObjectParser::getValue(
//    const ZJsonObject &obj, const std::vector<std::string> &candidateKeys,
//    const std::string &defaultValue);

#endif // ZJSONOBJECTPARSER_H
