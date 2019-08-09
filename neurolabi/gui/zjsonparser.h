#ifndef ZJSONPARSER_H
#define ZJSONPARSER_H

#include <vector>
#include <string>

#include "common/neutube_def.h"
#include "neurolabi_config.h"
#include "tz_json.h"
#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zjsonvalue.h"

class ZIntPoint;

class ZJsonParser
{
public:
  ZJsonParser();

public:
  /*!
   * \brief Decode a json string
   *
   * \param str A json string
   * \return A raw json object. The user is responsible for freeing it.
   */
  json_t* decode(const std::string &str);

  void printError() const;



//  template<typename T, typename std::enable_if<!std::is_integral<T>::value, int>::type = 0>
//  T getValue(const json_t *value) const;


  template<typename T, typename T2 = void>
  struct GetValueHelper {
    static T GetValue(const json_t *value);
  };

  template<typename T>
  struct GetValueHelper<T,
      neutu::void_t<typename std::enable_if<std::is_integral<T>::value, T>::type>> {
    static T GetValue(const json_t *value) {
      return integerValue(value);
    }
  };

//  T getValue<T,
//  void_t<typename std::enable_if<std::is_integral<T>::value, T>::type>>(const json_t *value) const
//  {
//    if (value == NULL) {
//      return m_defaultIntValue;
//    }

//    return integerValue(value);
//  }


  template<typename T>
  T getValue(const json_t *value) const {
    return GetValueHelper<T>::GetValue(value);
  }


//  template<>
//  static std::string getValue<std::string>(const json_t *value);

public:
  static bool IsObject(const std::string &str);

  static bool IsObject(const json_t *value);
  static bool IsArray(const json_t *value);
  static bool IsInteger(const json_t *value);
  static bool IsReal(const json_t *value);
  static bool IsNumber(const json_t *value);
  static bool IsBoolean(const json_t *value);

  //It returns 0 if <array> is not an array
  static size_t arraySize(const json_t *array);

  static json_t* arrayValue(const json_t *array, size_t index);
  static json_type type(const json_t *value);
  static void incref(json_t *value);
  static void decref(json_t *value);

  static std::string stringValue(const json_t *value);
  static double numberValue(const json_t *value);

  /*!
   * \brief Integer value of a json value.
   *
   * \return 0 if \a value is not in integer type or it is NULL.
   */
  static int64_t integerValue(const json_t *value);

  static bool booleanValue(const json_t *value);
  static bool booleanValue(const json_t *value, bool defaultValue);

  static std::string stringValue(const json_t *value, size_t index);
  static double numberValue(const json_t *value, size_t index);
  static int64_t integerValue(const json_t *value, size_t index);

  static std::vector<int64_t> integerArray(const json_t *value);

  static void print(const char *key, json_t *value, int indent);

  static ZIntPoint toIntPoint(const json_t *value);

private:
  json_error_t m_error;
//  std::string m_defaultStringValue;
//  int64_t m_defaultIntValue = 0;
//  bool m_defaultBoolValue = false;

//  const static char m_emptyString[1];
};

template<>
struct ZJsonParser::GetValueHelper<std::string> {
  static std::string GetValue(const json_t *value) {
    return stringValue(value);
  }
};

template<>
struct ZJsonParser::GetValueHelper<bool> {
  static bool GetValue(const json_t *value) {
    if (value == NULL) {
      return false;
    }

    return booleanValue(value);
  }
};

#endif // ZJSONPARSER_H
