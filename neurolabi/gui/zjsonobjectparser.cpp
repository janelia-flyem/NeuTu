#include "zjsonobjectparser.h"

ZJsonObjectParser::ZJsonObjectParser()
{
}

ZJsonObjectParser::ZJsonObjectParser(const ZJsonObject &obj)
{
  m_obj = obj;
}

template<>
ZJsonObject ZJsonObjectParser::GetValue<ZJsonObject>(
        const ZJsonObject &obj, const char *key)
{
  return ZJsonObject(obj.value(key));
}

template<>
std::string ZJsonObjectParser::GetValue<std::string>(
        const ZJsonObject &obj, const std::string &key)
{
  return GetValue(obj, key, std::string(""));
}

template<>
std::string ZJsonObjectParser::GetValue<std::string>(
        const ZJsonObject &obj, const char *key)
{
  return GetValue(obj, key, std::string(""));
}

//template<typename T>
//std::enable_if<std::is_integral<T>::value>::type
//ZJsonParser::getValue(const ZJsonObject &obj, const std::string &key,
//                      const T &defaultValue) const
//{
//    if (obj.hasKey(key.c_str())) {
//        return ZJsonParser().getValue<int64_t>(obj[key.c_str()]);
//    }

//    return defaultValue;
//}

std::string ZJsonObjectParser::GetValue(
        const ZJsonObject &obj, const char *key, const char *defaultValue)
{
   return GetValue(obj, key, std::string(defaultValue));
}

std::string ZJsonObjectParser::GetValue(
        const ZJsonObject &obj, const std::string &key, const char *defaultValue)
{
   return GetValue(obj, key.c_str(), defaultValue);
}

std::string ZJsonObjectParser::GetValue(
    const ZJsonObject &obj, const std::vector<std::string> &candidateKeys,
    const char* defaultValue)
{
  return GetValue(obj, candidateKeys, std::string(defaultValue));
}

std::string ZJsonObjectParser::getValue(
    const char *key, const char *defaultValue) const
{
   return getValue(key, std::string(defaultValue));
}

std::string ZJsonObjectParser::getValue(
    const std::string &key, const char *defaultValue) const
{
   return getValue(key.c_str(), defaultValue);
}
//std::string ZJsonObjectParser::getValue(
//    const ZJsonObject &obj, const std::vector<const char*> &candidateKeys,
//    const char* defaultValue)
//{
//  return getValue(obj, candidateKeys, std::string(defaultValue));
//}

