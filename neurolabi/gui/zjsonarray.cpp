#include "zjsonarray.h"
#include "c_json.h"
#include "tz_utilities.h"
#include "zjsonparser.h"
#include "zerror.h"

using namespace std;

ZJsonArray::ZJsonArray() : ZJsonValue()
{
}

ZJsonArray::ZJsonArray(json_t *data, bool asNew) : ZJsonValue()
{
  if (json_is_array(data)) {
    set(data, asNew);
  }
}

ZJsonArray::ZJsonArray(const json_t *data, bool asNew) : ZJsonValue()
{
  if (json_is_array(data)) {
    set(const_cast<json_t*>(data), asNew);
  }
}

ZJsonArray::ZJsonArray(json_t *data, ESetDataOption option) : ZJsonValue()
{
  if (json_is_array(data)) {
    set(data, option);
  }
}

ZJsonArray::ZJsonArray(const ZJsonValue &v) : ZJsonValue()
{
  if (v.isArray()) {
    set(v.getData(), ZJsonValue::SET_INCREASE_REF_COUNT);
  }
}

ZJsonArray::~ZJsonArray()
{

}

size_t ZJsonArray::size() const
{
  return C_Json::arraySize(m_data);
//  return json_array_size(m_data);
}

bool ZJsonArray::isEmpty() const
{
  return size() == 0;
}

json_t* ZJsonArray::at(size_t index)
{
  return json_array_get(m_data, index);
}

const json_t* ZJsonArray::at(size_t index) const
{
  return json_array_get(m_data, index);
}

void ZJsonArray::append(int v)
{
  append(json_integer(v));
}

void ZJsonArray::append(int64_t v)
{
  append(json_integer(v));
}

void ZJsonArray::append(uint64_t v)
{
  append(json_integer(v));
}

void ZJsonArray::append(double v)
{
  append(json_real(v));
}

void ZJsonArray::append(const char *str)
{
  append(json_string(str));
}

void ZJsonArray::append(const string &str)
{
  append(json_string(str.c_str()));
}

void ZJsonArray::append(json_t *obj)
{
  if (obj != NULL) {
    if (m_data == NULL) {
      m_data = json_array();
    }
    C_Json::appendArray(m_data, obj);
  }
}

void ZJsonArray::append(const ZJsonValue &obj)
{
  append(obj.getValue());
}

void ZJsonArray::remove(size_t index)
{
  json_array_remove(m_data, index);
}

void ZJsonArray::setValue(size_t i, const ZJsonValue &obj)
{
  json_array_set(m_data, i, obj.getData());
}

std::vector<double> ZJsonArray::toNumberArray() const
{
  std::vector<double> array;
  if (m_data != NULL) {
    for (size_t i = 0; i < size(); ++i) {
      const json_t *value = at(i);
      if (json_is_number(value)) {
        array.push_back(json_number_value(value));
      }
    }
  }

  return array;
}

std::vector<int> ZJsonArray::toIntegerArray() const
{
  std::vector<int> array;
  if (m_data != NULL) {
    for (size_t i = 0; i < size(); ++i) {
      const json_t *value = at(i);
      if (json_is_integer(value)) {
        array.push_back(json_integer_value(value));
      }
    }
  }

  return array;
}

std::vector<bool> ZJsonArray::toBoolArray() const
{
  std::vector<bool> array;
  if (m_data != NULL) {
    for (size_t i = 0; i < size(); ++i) {
      const json_t *value = at(i);
      if (json_is_boolean(value)) {
        array.push_back(json_is_true(value));
      }
    }
  }

  return array;
}

ZJsonArray& ZJsonArray::operator << (double e)
{
  if (m_data == NULL) {
    m_data = C_Json::makeArray();
  }

  if (Is_Integer_Value(e)) {
    C_Json::appendArray(m_data, json_integer(e));
  } else {
    C_Json::appendArray(m_data, json_real(e));
  }

  return *this;
}

bool ZJsonArray::decode(const string &str)
{
  clear();

  ZJsonParser parser;
  json_t *obj = parser.decode(str);

  if (ZJsonParser::isArray(obj)) {
    set(obj, true);
  } else {
    if (obj == NULL) {
      parser.printError();
    } else {
      json_decref(obj);
      RECORD_ERROR_UNCOND("Not a json array");
    }

    return false;
  }

  return true;
}

string ZJsonArray::dumpString(int indent) const
{
  if (isEmpty()) {
    return "[]";
  }

  return ZJsonValue::dumpString(indent);
}

ZJsonValue ZJsonArray::value(size_t index) const
{
  return ZJsonValue(at(index), ZJsonValue::SET_INCREASE_REF_COUNT);
}
