#include "zjsonparser.h"

#include <iostream>

#include "zintpoint.h"

using namespace std;

const char ZJsonParser::m_emptyString[] = {'\0' };

ZJsonParser::ZJsonParser()
{
}

bool ZJsonParser::IsObject(const json_t *value)
{
  if (value == NULL) {
    return false;
  }

  return json_is_object(value);
}

bool ZJsonParser::IsArray(const json_t *value)
{
  if (value == NULL) {
    return false;
  }

  return json_is_array(value);
}

bool ZJsonParser::IsInteger(const json_t *value)
{
  if (value == NULL) {
    return false;
  }

  return json_is_integer(value);
}

bool ZJsonParser::IsReal(const json_t *value)
{
  if (value == NULL) {
    return false;
  }

  return json_is_real(value);
}

bool ZJsonParser::IsNumber(const json_t *value)
{
  if (value == NULL) {
    return false;
  }

  return json_is_number(value);
}

bool ZJsonParser::IsBoolean(const json_t *value)
{
  if (value == NULL) {
    return false;
  }

  return json_is_boolean(value);
}

size_t ZJsonParser::arraySize(const json_t *array)
{
  if (!IsArray(array)) {
    return 0;
  }

  return json_array_size(array);
}

json_t* ZJsonParser::arrayValue(const json_t *array, size_t index)
{
  return json_array_get(array, index);
}

json_type ZJsonParser::type(const json_t *value)
{
  if (value == NULL) {
    return JSON_NULL;
  }

  return json_typeof(value);
}

void ZJsonParser::incref(json_t *value)
{
  json_incref(value);
}

void ZJsonParser::decref(json_t *value)
{
  json_decref(value);
}

const char* ZJsonParser::stringValue(const json_t *value)
{
  if (value == NULL || !json_is_string(value)) {
    return m_emptyString;
  }

  return json_string_value(value);
}

double ZJsonParser::numberValue(const json_t *value)
{
  return json_number_value(value);
}

int64_t ZJsonParser::integerValue(const json_t *value)
{
  if (value == NULL) {
    return 0;
  }

  return json_integer_value(value);
}

bool ZJsonParser::booleanValue(const json_t *value)
{
  if (json_is_true(value)) {
    return true;
  }

  return false;
}

bool ZJsonParser::booleanValue(const json_t *value, bool defaultValue)
{
  if (IsBoolean(value)) {
    return booleanValue(value);
  }

  return defaultValue;
}

const char* ZJsonParser::stringValue(const json_t *value, size_t index)
{
  return stringValue(arrayValue(value, index));
}

double ZJsonParser::numberValue(const json_t *value, size_t index)
{
  return numberValue(arrayValue(value, index));
}

int ZJsonParser::integerValue(const json_t *value, size_t index)
{
  return integerValue(arrayValue(value, index));
}

std::vector<int> ZJsonParser::integerArray(const json_t *value)
{
  std::vector<int> array;
  if (value != NULL) {
    if (IsArray(value)) {
      int s = arraySize(value);
      for (int i = 0; i < s; ++i) {
        json_t *a = ZJsonParser::arrayValue(value, i);
        if (a != NULL) {
          if (ZJsonParser::IsInteger(a)) {
            array.push_back(ZJsonParser::integerValue(a));
          }
        }
      }
    }
  }

  return array;
}

void ZJsonParser::print(const char *key, json_t *object, int indent)
{
  for (int i = 0; i < indent; ++i) {
    cout << " ";
  }

  if (key != NULL) {
    cout << key << ": ";
  }

  switch (type(object)) {
  case JSON_NULL:
    cout << "NULL" << endl;
    break;
  case JSON_OBJECT:
  {
    cout << "{" << endl;
    json_t *value;
    const char *key;
    json_object_foreach(object, key, value) {
      print(key, value, indent + 2);
    }
    for (int i = 0; i < indent; ++i) {
      cout << " ";
    }
    cout << "}" << endl;
  }
    break;
  case JSON_ARRAY:
  {
    int n = arraySize(object);
    cout << "[" << endl;
    for (int i = 0; i < n; ++i) {
      print(NULL, arrayValue(object, i), indent + 2);
    }
    for (int i = 0; i < indent; ++i) {
      cout << " ";
    }
    cout << "]" << endl;
  }
    break;
  case JSON_STRING:
    cout << stringValue(object) << endl;
    break;
  case JSON_INTEGER:
    cout << integerValue(object) << endl;
    break;
  case JSON_REAL:
    cout << numberValue(object) << endl;
    break;
  case JSON_TRUE:
    cout << "true" << endl;
    break;
  case JSON_FALSE:
    cout << "false" << endl;
    break;
  }
}

json_t* ZJsonParser::decode(const string &str)
{
  return json_loads(str.c_str(), 0, &m_error);
}


void ZJsonParser::printError() const
{
  ZJsonValue::PrintError(m_error);
#if 0
  std::cout << "Line " << m_error.line << " Column " << m_error.column
            << ": " << m_error.text << std::endl;
#endif
}

ZIntPoint ZJsonParser::toIntPoint(const json_t *value)
{
  ZIntPoint pt;
  if (value != NULL) {
    if (IsArray(value)) {
      if (arraySize(value) == 3) {
        pt.set(integerValue(value, 0), integerValue(value, 1), integerValue(value, 2));
      }
    }
  }

  return pt;
}

template<>
std::string ZJsonParser::getValue<std::string>(const json_t *value) const
{
  if (value == NULL) {
    return m_defaultStringValue;
  }

  return stringValue(value);
}

template<>
bool ZJsonParser::getValue<bool>(const json_t *value) const
{
  if (value == NULL) {
    return m_defaultBoolValue;
  }

  return booleanValue(value);
}
