#include "zjsonarray.h"
#include "c_json.h"
#include "tz_utilities.h"
#include "zjsonparser.h"
#include "zerror.h"

using namespace std;

ZJsonArray::ZJsonArray() : ZJsonValue()
{
}

ZJsonArray::ZJsonArray(json_t *data, bool asNew)
{
  if (json_is_array(data)) {
    set(data, asNew);
  } else {
    m_data = NULL;
  }
}

ZJsonArray::~ZJsonArray()
{

}

size_t ZJsonArray::size() const
{
  return json_array_size(m_data);
}

json_t* ZJsonArray::at(size_t index)
{
  return json_array_get(m_data, index);
}

const json_t* ZJsonArray::at(size_t index) const
{
  return json_array_get(m_data, index);
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

void ZJsonArray::append(ZJsonValue &obj)
{
  append(obj.getValue());
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
