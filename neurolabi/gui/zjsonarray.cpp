#include "zjsonarray.h"
#include "c_json.h"

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
  if (m_data != NULL && obj != NULL) {
    C_Json::appendArray(m_data, obj);
  }
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
