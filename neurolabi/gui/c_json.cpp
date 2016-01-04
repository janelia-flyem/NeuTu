#include "c_json.h"
#include <vector>
#include <iostream>

const static char* JSON_EMPTY_STRING = "";

json_t* C_Json::makeObject()
{
  return json_object();
}

json_t* C_Json::makeInteger(int v)
{
  return json_integer(v);
}

json_t* C_Json::makeNumber(double v)
{
  return json_real(v);
}

json_t* C_Json::makeString(const char *v)
{
  return json_string(v);
}

json_t* C_Json::makeString(const std::string &v)
{
  return makeString(v.c_str());
}

json_t* C_Json::makeBoolean(bool v)
{
  return v ? json_true() : json_false();
}

json_t* C_Json::makeArray()
{
  return json_array();
}

json_t* C_Json::makeArray(const double *array, size_t n)
{
  if (array == NULL || n <= 0) {
    return NULL;
  }

  json_t *obj = makeArray();
  for (size_t i = 0; i < n; ++i) {
    appendArray(obj, makeNumber(array[i]));
  }

  return obj;
}

json_t* C_Json::makeArray(const int *array, size_t n)
{
  if (array == NULL || n <= 0) {
    return NULL;
  }

  json_t *obj = makeArray();
  for (size_t i = 0; i < n; ++i) {
    appendArray(obj, makeInteger(array[i]));
  }

  return obj;
}

void C_Json::appendArray(json_t *array, json_t *v)
{
  json_array_append(array, v);
}

void C_Json::decref(json_t *json)
{
  json_decref(json);
}

void C_Json::incref(json_t *json)
{
  json_incref(json);
}

bool C_Json::dump(const json_t *obj, const char *filePath)
{
  if (obj == NULL || filePath == NULL) {
    return false;
  }

  if (json_dump_file(obj, filePath, JSON_INDENT(2)) != 0) {
    return false;
  }

  return true;
}

bool C_Json::isObject(const json_t *value)
{
  if (value == NULL) {
    return false;
  }

  return json_is_object(value);
}

bool C_Json::isArray(const json_t *value)
{
  if (value == NULL) {
    return false;
  }

  return json_is_array(value);
}

bool C_Json::isInteger(const json_t *value)
{
  if (value == NULL) {
    return false;
  }

  return json_is_integer(value);
}

bool C_Json::isReal(const json_t *value)
{
  if (value == NULL) {
    return false;
  }

  return json_is_real(value);
}

bool C_Json::isNumber(const json_t *value)
{
  if (value == NULL) {
    return false;
  }

  return json_is_number(value);
}

bool C_Json::isBoolean(const json_t *value)
{
  if (value == NULL) {
    return false;
  }

  return json_is_boolean(value);
}

size_t C_Json::arraySize(const json_t *array)
{
  if (!isArray(array)) {
    return 0;
  }

  return json_array_size(array);
}

json_t* C_Json::arrayValue(const json_t *array, size_t index)
{
  return json_array_get(array, index);
}

json_type C_Json::type(const json_t *value)
{
  return json_typeof(value);
}

const char* C_Json::stringValue(const json_t *value)
{
  if (value == NULL || !json_is_string(value)) {
    return JSON_EMPTY_STRING;
  }

  return json_string_value(value);
}

double C_Json::numberValue(const json_t *value)
{
  return json_number_value(value);
}

int64_t C_Json::integerValue(const json_t *value)
{
  if (value == NULL) {
    return 0;
  }

  return json_integer_value(value);
}

bool C_Json::booleanValue(const json_t *value)
{
  if (json_is_true(value)) {
    return true;
  }

  return false;
}

const char* C_Json::stringValue(const json_t *value, size_t index)
{
  return stringValue(arrayValue(value, index));
}

double C_Json::numberValue(const json_t *value, size_t index)
{
  return numberValue(arrayValue(value, index));
}

int C_Json::integerValue(const json_t *value, size_t index)
{
  return integerValue(arrayValue(value, index));
}

std::vector<int> C_Json::integerArray(const json_t *value)
{
  std::vector<int> array;
  if (value != NULL) {
    if (isArray(value)) {
      int s = arraySize(value);
      for (int i = 0; i < s; ++i) {
        json_t *a = arrayValue(value, i);
        if (a != NULL) {
          if (isInteger(a)) {
            array.push_back(integerValue(a));
          }
        }
      }
    }
  }

  return array;
}

void C_Json::print(const char *key, json_t *object, int indent)
{
  for (int i = 0; i < indent; ++i) {
    std::cout << " ";
  }

  if (key != NULL) {
    std::cout << key << ": ";
  }

  switch (type(object)) {
  case JSON_NULL:
    std::cout << "NULL" << std::endl;
    break;
  case JSON_OBJECT:
  {
    std::cout << "{" << std::endl;
    json_t *value;
    const char *key;
    json_object_foreach(object, key, value) {
      print(key, value, indent + 2);
    }
    for (int i = 0; i < indent; ++i) {
      std::cout << " ";
    }
    std::cout << "}" << std::endl;
  }
    break;
  case JSON_ARRAY:
  {
    int n = arraySize(object);
    std::cout << "[" << std::endl;
    for (int i = 0; i < n; ++i) {
      print(NULL, arrayValue(object, i), indent + 2);
    }
    for (int i = 0; i < indent; ++i) {
      std::cout << " ";
    }
    std::cout << "]" << std::endl;
  }
    break;
  case JSON_STRING:
    std::cout << stringValue(object) << std::endl;
    break;
  case JSON_INTEGER:
    std::cout << integerValue(object) << std::endl;
    break;
  case JSON_REAL:
    std::cout << numberValue(object) << std::endl;
    break;
  case JSON_TRUE:
    std::cout << "true" << std::endl;
    break;
  case JSON_FALSE:
    std::cout << "false" << std::endl;
    break;
  }
}

json_t* C_Json::clone(json_t *value)
{
  return json_deep_copy(value);
}
