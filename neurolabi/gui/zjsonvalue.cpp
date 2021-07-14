#include "zjsonvalue.h"

#include <sstream>
#include <iostream>
#include <cstring>

#include "zjsonparser.h"
#include "c_json.h"

using namespace std;

ZJsonValue::ZJsonValue() : m_data(NULL)
{
}

/*
ZJsonValue::ZJsonValue(json_t *data, bool asNew) : m_data(NULL)
{
  set(data, asNew);
}
*/

ZJsonValue::ZJsonValue(json_t *data, ESetDataOption option) : m_data(NULL)
{
  set(data, option);
#ifdef _DEBUG_2
      std::cout << m_data << std::endl;
#endif
}

ZJsonValue::ZJsonValue(const json_t *data, ESetDataOption option) : m_data(NULL)
{
  set(const_cast<json_t*>(data), option);
#ifdef _DEBUG_2
      std::cout << m_data << std::endl;
#endif
}

ZJsonValue::ZJsonValue(const ZJsonValue &value) : m_data(NULL)
{
  set(value.m_data, SET_INCREASE_REF_COUNT);
}

ZJsonValue::ZJsonValue(int data)
{
  m_data = json_integer(data);
}

ZJsonValue::ZJsonValue(double data)
{
  m_data = json_real(data);
}

ZJsonValue::ZJsonValue(const char *data)
{
  m_data = json_string(data);
}

ZJsonValue::~ZJsonValue()
{
  if (m_data != NULL) {
    json_decref(m_data);
  }
}

bool ZJsonValue::isObject() const
{
  if (m_data == NULL) {
    return false;
  }

  return json_is_object(m_data);
}

bool ZJsonValue::isArray() const
{
  if (m_data == NULL) {
    return false;
  }

  return json_is_array(m_data);
}

bool ZJsonValue::isString() const
{
  if (m_data == NULL) {
    return false;
  }

  return json_is_string(m_data);
}

bool ZJsonValue::isInteger() const
{
  if (m_data == NULL) {
    return false;
  }

  return json_is_integer(m_data);
}

bool ZJsonValue::isReal() const
{
  if (m_data == NULL) {
    return false;
  }

  return json_is_real(m_data);
}

bool ZJsonValue::isNumber() const
{
  if (m_data == NULL) {
    return false;
  }

  return json_is_number(m_data);
}

bool ZJsonValue::isBoolean() const
{
  if (m_data == NULL) {
    return false;
  }

  return json_is_boolean(m_data);
}

bool ZJsonValue::isEmpty() const
{
  return m_data == NULL || json_is_null(m_data);
}

bool ZJsonValue::isNull() const
{
  return m_data == NULL;
}

std::string ZJsonValue::getType() const
{
  if (isNull()) {
    return "null";
  } else if (isObject()) {
    return "object";
  } else if (isArray()) {
    return "array";
  } else if (isInteger()) {
    return "int";
  } else if (isReal()) {
    return "real";
  } else if (isBoolean()) {
    return "boolean";
  }

  return "undefined";
}

void ZJsonValue::set(const ZJsonValue &value)
{
  set(value.getData(), ZJsonValue::SET_INCREASE_REF_COUNT);
}

void ZJsonValue::set(json_t *data, bool asNew)
{
  if (m_data == data) {
    return;
  }

  if (m_data != NULL) {
    json_decref(m_data);
  }

  m_data = data;

  if ((!asNew) && (data != NULL)) {
    json_incref(data);
  }
}

void ZJsonValue::set(json_t *data, ESetDataOption option)
{
  if (m_data != NULL) {
    json_decref(m_data);
    m_data = NULL;
  }

  if (data != NULL) {
    switch (option) {
    case SET_INCREASE_REF_COUNT:
      json_incref(data);
      m_data = data;
      break;
    case SET_AS_IT_IS:
      m_data = data;
      break;
    case SET_DEEP_COPY:
      m_data = json_deep_copy(data);
      break;
    case SET_SHALLOW_COPY:
      m_data = json_copy(data);
      break;
    }
  }
}

void ZJsonValue::decodeString(const char *str)
{
  json_error_t error;
  decodeString(str, &error);
}

void ZJsonValue::decodeString(const char *str, json_error_t *error)
{
  clear();

  if (str && strlen(str) > 0) {
    //  json_error_t error;
    json_error_t *ownError = NULL;
    if (error == NULL) {
      error = (ownError = new json_error_t);
    }

    m_data = json_loads(str, JSON_DECODE_ANY, error);

    delete ownError;
  }
}

void ZJsonValue::clear()
{
  if (m_data != NULL) {
    json_decref(m_data);
  }

  m_data = NULL;
}

void ZJsonValue::print() const
{
//  std::cout << dumpString(2);

  ZJsonParser::print(NULL, m_data, 0);
}

ZJsonValue& ZJsonValue::operator = (const ZJsonValue &value)
{
  set(value.m_data, false);

  return (*this);
}

std::vector<ZJsonValue> ZJsonValue::toArray()
{
  std::vector<ZJsonValue> array;

  if (isArray()) {
    json_t *value = getValue();
    size_t n = ZJsonParser::ArraySize(value);
    array.resize(n);
    for (size_t i = 0; i < n; ++i) {
      array[i].set(ZJsonParser::ArrayValue(value, i), false);
    }
  }
  return array;
}

void ZJsonValue::PrintError(const json_error_t &error)
{
  if (error.line > 0) {
    std::cout << "JSON decoding error: " << std::endl;
    std::cout << "  " << GetErrorString(error) << std::endl;
  }
}

std::string ZJsonValue::GetErrorString(const json_error_t &error)
{
  ostringstream stream;
  stream << "Line " << error.line << " Column " << error.column
         << ": " << error.text;

  return stream.str();
}

int ZJsonValue::toInteger() const
{
  return ZJsonParser::integerValue(m_data);
}

double ZJsonValue::toReal() const
{
  return ZJsonParser::numberValue(m_data);
}

std::string ZJsonValue::toString() const
{
  return ZJsonParser::stringValue(m_data);
}

bool ZJsonValue::toBoolean() const
{
  return ZJsonParser::booleanValue(m_data);
}
/*
std::string ZJsonValue::toString() const
{
  return ZJsonParser::stringValue(m_data);
}
*/

std::string ZJsonValue::dumpString(int indent) const
{
  return dumpJanssonString(JSON_INDENT(indent) | JSON_PRESERVE_ORDER);
}

std::string ZJsonValue::dumpJanssonString(size_t flags) const
{
  string str;
  if (!isEmpty()) {
    char *cstr = json_dumps(getValue(), flags);
    str = cstr;
    free(cstr);
  }

  return str;
}

bool ZJsonValue::dump(const string &path) const
{
#ifdef _DEBUG_
  std::cout << "Saving json file: " << path << " ..." << std::endl;
#endif
  return C_Json::dump(m_data, path.c_str());
}

bool ZJsonValue::load(const string &filePath)
{
#if defined(HAVE_LIBJANSSON)
  if (m_data != NULL) {
    json_decref(m_data);
  }

  json_error_t error;
  m_data = json_load_file(filePath.c_str(), 0, &error);
  if (m_data) {
    m_source = filePath;
    return true;
  }

  std::cout << filePath << "(" << error.line << ")" << ": "
            << error.text << std::endl;
#endif

  return false;
}

std::string ZJsonValue::getSource() const
{
  return m_source;
}

ZJsonValue ZJsonValue::clone() const
{
  return ZJsonValue(C_Json::clone(m_data), SET_AS_IT_IS);
}

void ZJsonValue::denull()
{
  if (m_data == NULL) {
    m_data = C_Json::makeJsonNull();
  }
}

int ZJsonValue::getRefCount() const
{
  if (m_data == NULL) {
    return 0;
  }

  return m_data->refcount;
}

std::ostream& operator << (std::ostream &stream, const ZJsonValue &v)
{
  stream << v.dumpString(0);
  return stream;
}
