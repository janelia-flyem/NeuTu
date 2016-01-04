#include "zjsonvalue.h"

#include <sstream>
#include <iostream>

#include "zjsonparser.h"
#include "c_json.h"

using namespace std;

ZJsonValue::ZJsonValue() : m_data(NULL)
{
}

ZJsonValue::ZJsonValue(json_t *data, bool asNew) : m_data(NULL)
{
  set(data, asNew);
}

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

  return json_is_object(m_data) > 0;
}

bool ZJsonValue::isArray() const
{
  if (m_data == NULL) {
    return false;
  }

  return json_is_array(m_data) > 0;
}

bool ZJsonValue::isString() const
{
  if (m_data == NULL) {
    return false;
  }

  return json_is_string(m_data) > 0;
}

bool ZJsonValue::isInteger()
{
  if (m_data == NULL) {
    return false;
  }

  return json_is_integer(m_data) > 0;
}

bool ZJsonValue::isReal()
{
  if (m_data == NULL) {
    return false;
  }

  return json_is_real(m_data) > 0;
}

bool ZJsonValue::isNumber()
{
  if (m_data == NULL) {
    return false;
  }

  return json_is_number(m_data) > 0;
}

bool ZJsonValue::isBoolean()
{
  if (m_data == NULL) {
    return false;
  }

  return json_is_boolean(m_data) > 0;
}

bool ZJsonValue::isEmpty() const
{
  return m_data == NULL;
}

void ZJsonValue::set(const ZJsonValue &value)
{
  set(value.getData(), ZJsonValue::SET_INCREASE_REF_COUNT);
}

void ZJsonValue::set(json_t *data, bool asNew)
{
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
  if (m_data != NULL) {
    json_decref(m_data);
  }

  m_data = json_loads(str, JSON_DECODE_ANY, &m_error);
  if (m_error.line > 0) {
    std::cout << m_error.source << std::endl;
    std::cout << m_error.text << std::endl;
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
    size_t n = ZJsonParser::arraySize(value);
    array.resize(n);
    for (size_t i = 0; i < n; ++i) {
      array[i].set(ZJsonParser::arrayValue(value, i), false);
    }
  }
  return array;
}

std::string ZJsonValue::getErrorString() const
{
  ostringstream stream;
  stream << "Line " << m_error.line << " Column " << m_error.column
         << ": " << m_error.text;

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

/*
std::string ZJsonValue::toString() const
{
  return ZJsonParser::stringValue(m_data);
}
*/

std::string ZJsonValue::dumpString(int indent) const
{
  string str;
  if (!isEmpty()) {
    char *cstr = json_dumps(getValue(), JSON_INDENT(indent));
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

  m_data = json_load_file(filePath.c_str(), 0, &m_error);
  if (m_data) {
    return true;
  }


  std::cout << filePath << "(" << m_error.line << ")" << ": "
            << m_error.text << std::endl;
#endif

  return false;
}

ZJsonValue ZJsonValue::clone() const
{
  return ZJsonValue(C_Json::clone(m_data), SET_AS_IT_IS);
}
