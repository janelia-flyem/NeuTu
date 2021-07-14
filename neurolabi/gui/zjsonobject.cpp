#include "zjsonobject.h"

#include <iostream>
#include <sstream>
#include <string>
#include <cstdarg>

#include "zjsonparser.h"
#include "c_json.h"
#include "zerror.h"

using namespace std;

/*
ZJsonObject::ZJsonObject(json_t *json, bool asNew) : ZJsonValue()
{
  if (ZJsonParser::isObject(json)) {
    set(json, asNew);
  }
}
*/

ZJsonObject::ZJsonObject(json_t *data, ESetDataOption option) : ZJsonValue()
{
  if (ZJsonParser::IsObject(data)) {
    set(data, option);
  }
}

ZJsonObject::ZJsonObject()
{
}

ZJsonObject::ZJsonObject(const ZJsonObject &obj) : ZJsonValue(obj)
{
}

ZJsonObject::ZJsonObject(const ZJsonValue &obj)
{
  if (obj.isObject()) {
    set(obj.getData(), ZJsonValue::SET_INCREASE_REF_COUNT);
  }
}

ZJsonObject::~ZJsonObject()
{
#ifdef _DEBUG_2
  std::cout << "Destroying " << this << std::endl;
  std::cout << dumpString() << std::endl;
#endif
}

bool ZJsonObject::hasKey(const char *key) const
{
  return (*this)[key] != NULL;
}

bool ZJsonObject::hasKey(const string &key) const
{
  return hasKey(key.c_str());
}

json_t* ZJsonObject::operator[] (const char *key)
{
  return const_cast<json_t*>(static_cast<const ZJsonObject&>(*this)[key]);
}

bool ZJsonObject::isEmpty() const
{
  if (m_data == NULL) {
    return true;
  }

  return json_object_size(m_data) == 0;
}

void ZJsonObject::denull()
{
  if (m_data == NULL) {
    m_data = C_Json::makeObject();
  }
}

const json_t* ZJsonObject::operator[] (const char *key) const
{
  if (m_data != NULL) {
    const char *qkey;
    json_t *value;

    json_object_foreach(m_data, qkey, value) {
      if (strcmp(qkey, key) == 0) {
        return value;
      }
    }
  }

  return NULL;
}

ZJsonValue ZJsonObject::value(const char *key) const
{
  return ZJsonValue(const_cast<json_t*>((*this)[key]),
                    ZJsonValue::SET_INCREASE_REF_COUNT);
}

ZJsonValue ZJsonObject::value(const string &key) const
{
  return value(key.c_str());
}

#ifndef SWIG
ZJsonValue ZJsonObject::value(
    const std::initializer_list<const char*> &keyList) const
{
  ZJsonValue v = *this;
  for (const char *key : keyList) {
    ZJsonObject obj(v);
    v = obj.value(key);
    if (v.isEmpty()) {
      break;
    }
  }

  return v;
}
#endif

bool ZJsonObject::decode(const string &str, bool reportingError)
{
  clear();
  if (str.empty()) {
#ifdef _DEBUG_
      std::cout << "JSON decoding: Nothing to decode." << std::endl;
#endif
    return false;
  }

  ZJsonParser parser;
  json_t *obj = parser.decode(str);

  if (ZJsonParser::IsObject(obj)) {
    set(obj, true);
  } else {
    if (obj == NULL && reportingError) {
      parser.printError();
    } else {
      json_decref(obj);

      if (reportingError) {
        std::cout << "Not a json object" << std::endl;
      }
    }

    return false;
  }

  return true;
}

string ZJsonObject::summary()
{
  ostringstream stream;

  if (m_data != NULL) {
    const char *key;
    json_t *value;

    json_object_foreach(m_data, key, value) {
      if (json_is_object(value)) {
        ZJsonObject obj(value, ZJsonValue::SET_INCREASE_REF_COUNT);
        stream << obj.summary();
      } else if (json_is_array(value)) {
        stream << key << " : " << "Array " << endl;
        for (size_t i = 0; i < json_array_size(value); i++) {
          json_t *element = json_array_get(value, i);
          if (json_is_object(element)) {
            ZJsonObject obj(element, ZJsonValue::SET_INCREASE_REF_COUNT);
            stream << obj.summary();
          } else {
            stream << "Element : " << "Type " << json_typeof(element) << endl;
          }
        }
      } else {
        stream << key << " : " << "Type " << json_typeof(value) << endl;
      }
    }
  }

  return stream.str();
}

void ZJsonObject::appendEntries(const char *key, json_t *obj,
                                map<string, json_t*> *entryMap)
{
  if (key != NULL && obj != NULL) {
    (*entryMap)[key] = obj;
  }

  if (ZJsonParser::IsObject(obj)) {
    const char *subkey = NULL;
    json_t *value = NULL;
    json_object_foreach(obj, subkey, value) {
      appendEntries(subkey, value, entryMap);
    }
  }
}

map<string, json_t*> ZJsonObject::toEntryMap(bool recursive) const
{
//  TZ_ASSERT(json_is_object(m_data), "Invalid json object");


  map<std::string, json_t*> entryMap;
  if (C_Json::isObject(m_data)) {
    if (recursive) {
      appendEntries(NULL, m_data, &entryMap);
    } else {
      const char *subkey = NULL;
      json_t *value = NULL;
      json_object_foreach(m_data, subkey, value) {
        entryMap[subkey] = value;
      }
    }
  }

  return entryMap;
}


bool ZJsonObject::isValidKey(const char *key)
{
  if (key == NULL) {
    return false;
  }

  if (key[0] == '\0') {
    return false;
  }

  return true;
}

void ZJsonObject::setEntryWithoutKeyCheck(
    const char *key, json_t *obj, bool asNew)
{
  if (obj != NULL) {
    if (m_data == NULL) {
      m_data = C_Json::makeObject();
    }
    if (asNew) {
      json_object_set_new(m_data, key, obj);
    } else {
      json_object_set(m_data, key, obj);
    }
  }
}

ZJsonObject &ZJsonObject::setEntry(const char *key, json_t *obj)
{
  if (isValidKey(key)) {
    setEntryWithoutKeyCheck(key, obj);
  }

  return *this;
}

ZJsonObject& ZJsonObject::setEntry(const std::string &key, json_t *obj)
{
  return setEntry(key.c_str(), obj);
}

void ZJsonObject::consumeEntry(const char *key, json_t *obj)
{
  if (!isValidKey(key)) {
    if (obj != NULL) {
      json_decref(obj);
    }
    return;
  }

  setEntryWithoutKeyCheck(key, obj, true);
}

void ZJsonObject::setNonEmptyEntry(const char *key, const string &value)
{
  if (!value.empty()) {
    setEntry(key, value);
  }
}

void ZJsonObject::setNonEmptyEntry(const char *key, const ZJsonObject &obj)
{
  if (!obj.isEmpty() && isValidKey(key)) {
    setEntryWithoutKeyCheck(key, obj.getValue());
  }
}

void ZJsonObject::setNonEmptyEntry(const char *key, const ZJsonArray &obj)
{
  if (!obj.isEmpty() && isValidKey(key)) {
    setEntryWithoutKeyCheck(key, obj.getValue());
  }
}

ZJsonObject &ZJsonObject::setEntry(const char *key, const string &value)
{
  if (isValidKey(key)) {
    setEntryWithoutKeyCheck(key, json_string(value.c_str()), true);
  }

  return *this;
}

ZJsonObject &ZJsonObject::setEntry(const char *key, const char *value)
{
  if (isValidKey(key) && value != NULL) {
    setEntryWithoutKeyCheck(key, json_string(value), true);
  }

  return *this;
}

ZJsonObject &ZJsonObject::setEntry(const string &key, const string &value)
{
  return setEntry(key.c_str(), value.c_str());
}

ZJsonObject& ZJsonObject::setEntry(const std::string &key, const char *value)
{
  return setEntry(key.c_str(), value);
}

ZJsonObject& ZJsonObject::setEntry(
    const char *key, const std::vector<string> &value)
{
  if (!value.empty()) {
    ZJsonArray jsonArray;
    for (const std::string &v : value) {
      if (!v.empty()) {
        jsonArray.append(v);
      }
    }
    setEntry(key, jsonArray);
  }

  return *this;
}

ZJsonObject& ZJsonObject::setEntry(const char *key, const double *array, size_t n)
{
  if (isValidKey(key)) {
    setEntryWithoutKeyCheck(key, C_Json::makeArray(array, n), true);
  }

  return *this;
}

ZJsonObject& ZJsonObject::setEntry(const char *key, const std::vector<double> &value)
{
  return setEntry(key, value.data(), value.size());
}


ZJsonObject &ZJsonObject::setEntry(const char *key, const int *array, size_t n)
{
  if (isValidKey(key)) {
    setEntryWithoutKeyCheck(key, C_Json::makeArray(array, n), true);
  }

  return *this;
}

ZJsonObject &ZJsonObject::setEntry(const char *key, bool v)
{
  if (isValidKey(key)) {
    setEntryWithoutKeyCheck(key, C_Json::makeBoolean(v));
  }

  return *this;
}

ZJsonObject& ZJsonObject::setEntry(const std::string &key, bool v)
{
  setEntry(key.c_str(), v);

  return *this;
}

void ZJsonObject::setTrueEntry(const char *key, bool v)
{
  if (v) {
    setEntry(key, v);
  }
}

ZJsonObject& ZJsonObject::setEntry(const char *key, int64_t v)
{
  if (isValidKey(key)) {
    setEntryWithoutKeyCheck(key, C_Json::makeInteger(v), true);
  }

  return *this;
}

ZJsonObject& ZJsonObject::setEntry(const char *key, uint64_t v)
{
  if (isValidKey(key)) {
    setEntryWithoutKeyCheck(key, C_Json::makeInteger(v), true);
  }

  return *this;
}

ZJsonObject& ZJsonObject::setEntry(const char *key, int v)
{
  if (isValidKey(key)) {
    setEntryWithoutKeyCheck(key, C_Json::makeInteger(v), true);
  }

  return *this;
}

ZJsonObject& ZJsonObject::setEntry(const std::string &key, int64_t v)
{
  return setEntry(key.c_str(), v);
}

ZJsonObject& ZJsonObject::setEntry(const std::string &key, uint64_t v)
{
  return setEntry(key.c_str(), v);
}

ZJsonObject& ZJsonObject::setEntry(const std::string &key, int v)
{
  return setEntry(key.c_str(), v);
}

ZJsonObject& ZJsonObject::setEntry(const char *key, double v)
{
  if (isValidKey(key)) {
    setEntryWithoutKeyCheck(key, C_Json::makeNumber(v), true);
  }

  return *this;
}

ZJsonObject& ZJsonObject::setEntry(const char *key, ZJsonValue &value)
{
  if (isValidKey(key)) {
    value.denull();
    setEntryWithoutKeyCheck(key, value.getValue());
  }

  return *this;
}

ZJsonObject& ZJsonObject::setEntry(const std::string &key, ZJsonValue &value)
{
  return setEntry(key.c_str(), value);
}

void ZJsonObject::setEntry(const char *key, const ZJsonValue &value)
{
  if (!isValidKey(key)) {
    return;
  }

  setEntryWithoutKeyCheck(key, value.getValue());
}

void ZJsonObject::setEntry(const std::string &key, const ZJsonValue &value)
{
  setEntry(key.c_str(), value);
}

void ZJsonObject::addEntry(const char *key, ZJsonValue &value)
{
  if (!hasKey(key)) {
    setEntry(key, value);
  }
}

void ZJsonObject::addEntry(const char *key, const string &value)
{
  if (!hasKey(key)) {
    setEntry(key, value);
  }
}

void ZJsonObject::addEntry(const char *key, const char *value)
{
  if (!hasKey(key)) {
    setEntry(key, value);
  }
}

void ZJsonObject::addEntryFrom(const ZJsonObject &obj)
{
  const char *key;
  json_t *value;
  ZJsonObject_foreach(obj, key, value) {
    ZJsonValue valueJson(value, ZJsonValue::SET_INCREASE_REF_COUNT);
    addEntry(key, valueJson);
  }
}

json_t* ZJsonObject::setArrayEntry(const char *key)
{
  json_t* arrayObj = NULL;
  if (isValidKey(key)) {
    if (!hasKey(key)) {
      arrayObj = json_array();
      setEntryWithoutKeyCheck(key, arrayObj, true);
    }
  }

  return arrayObj;
}

void ZJsonObject::setValue(const ZJsonValue &value)
{
  if (value.isObject()) {
    set(value.getData(), false);
  } else {
    clear();
  }
}

std::vector<std::string> ZJsonObject::getAllKey() const
{
  std::vector<std::string> keyList;
  if (!isEmpty()) {
    const char *key;
    json_t *value;
    json_object_foreach(m_data, key, value) {
      keyList.push_back(key);
    }
  }

  return keyList;
}

void ZJsonObject::removeKey(const char *key)
{
  if (!isEmpty() && key != NULL) {
    json_object_del(m_data, key);
  }
}

void ZJsonObject::forEachValue(
    std::function<void(const std::string &key, ZJsonValue)> f) const
{
  if (!isEmpty()) {
    const char *key;
    json_t *value;
    json_object_foreach(m_data, key, value) {
      f(key, ZJsonValue(value, ESetDataOption::SET_INCREASE_REF_COUNT));
    }
  }
}

void ZJsonObject::forEachValue(std::function<void (ZJsonValue)> f) const
{
  forEachValue([&](const std::string &, ZJsonValue v) {
    f(v);
  });
}

bool ZJsonObject::all(
    std::function<bool(const std::string &key, ZJsonValue)> f) const
{
  if (!isEmpty()) {
    const char *key;
    json_t *value;
    json_object_foreach(m_data, key, value) {
      if (!f(key, ZJsonValue(value, ESetDataOption::SET_INCREASE_REF_COUNT))) {
        return false;
      }
    }
  }

  return true;
}

bool ZJsonObject::all(
    std::function<bool(const std::string &key)> f) const
{
  if (!isEmpty()) {
    const char *key;
    json_t *value;
    json_object_foreach(m_data, key, value) {
      if (!f(key)) {
        return false;
      }
    }
  }

  return true;
}

std::string ZJsonObject::dumpJanssonString(size_t flags) const
{
  if (isEmpty()) {
    return "{}";
  }

  return ZJsonValue::dumpJanssonString(flags);
}
