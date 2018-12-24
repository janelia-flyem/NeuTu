#include "zjsonobjectparser.h"

#include "zjsonparser.h"
#include "zjsonobject.h"

ZJsonObjectParser::ZJsonObjectParser()
{
}



template<>
ZJsonObject ZJsonObjectParser::getValue<ZJsonObject>(
        const ZJsonObject &obj, const char *key) const
{
  return ZJsonObject(obj.value(key));
}

template<>
std::string ZJsonObjectParser::getValue<std::string>(
        const ZJsonObject &obj, const std::string &key) const
{
  return getValue(obj, key, std::string(""));
}

template<>
std::string ZJsonObjectParser::getValue<std::string>(
        const ZJsonObject &obj, const char *key) const
{
  return getValue(obj, key, std::string(""));
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

std::string ZJsonObjectParser::getValue(
        const ZJsonObject &obj, const char *key, const char *defaultValue)
{
   return getValue(obj, key, std::string(defaultValue));
}

std::string ZJsonObjectParser::getValue(
        const ZJsonObject &obj, const std::string &key, const char *defaultValue)
{
   return getValue(obj, key.c_str(), defaultValue);
}
