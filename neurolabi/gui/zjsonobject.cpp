#include "zjsonobject.h"

#include <iostream>
#include <sstream>
#include <string.h>

#include "tz_error.h"
#include "zjsonparser.h"
#include "c_json.h"
#include "zerror.h"

using namespace std;

ZJsonObject::ZJsonObject(json_t *json, bool asNew) : ZJsonValue()
{
  if (ZJsonParser::isObject(json)) {
    set(json, asNew);
  }
}

ZJsonObject::ZJsonObject(json_t *data, ESetDataOption option) : ZJsonValue()
{
  if (ZJsonParser::isObject(data)) {
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

bool ZJsonObject::decode(const string &str)
{
  clear();

  ZJsonParser parser;
  json_t *obj = parser.decode(str);

  if (ZJsonParser::isObject(obj)) {
    set(obj, true);
  } else {
    if (obj == NULL) {
      parser.printError();
    } else {
      json_decref(obj);
      RECORD_ERROR_UNCOND("Not a json object");
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
        ZJsonObject obj(value, false);
        stream << obj.summary();
      } else if (json_is_array(value)) {
        stream << key << " : " << "Array " << endl;
        for (size_t i = 0; i < json_array_size(value); i++) {
          json_t *element = json_array_get(value, i);
          if (json_is_object(element)) {
            ZJsonObject obj(element, false);
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

  if (ZJsonParser::isObject(obj)) {
    const char *subkey = NULL;
    json_t *value = NULL;
    json_object_foreach(obj, subkey, value) {
      appendEntries(subkey, value, entryMap);
    }
  }
}

map<string, json_t*> ZJsonObject::toEntryMap(bool recursive) const
{
  TZ_ASSERT(json_is_object(m_data), "Invalid json object");

  map<std::string, json_t*> entryMap;

  if (recursive) {
    appendEntries(NULL, m_data, &entryMap);
  } else {
    const char *subkey = NULL;
    json_t *value = NULL;
    json_object_foreach(m_data, subkey, value) {
      entryMap[subkey] = value;
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

void ZJsonObject::setEntry(const char *key, json_t *obj)
{
  if (!isValidKey(key)) {
    return;
  }

  setEntryWithoutKeyCheck(key, obj);
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

void ZJsonObject::setEntry(const char *key, const string &value)
{
  if (!isValidKey(key)) {
    return;
  }

  if (!value.empty()) {
    setEntryWithoutKeyCheck(key, json_string(value.c_str()), true);
  }
}

void ZJsonObject::setEntry(const char *key, const char* value)
{
  if (!isValidKey(key)) {
    return;
  }

  if (value != NULL) {
    if (strlen(value) > 0) {
      setEntryWithoutKeyCheck(key, json_string(value), true);
    }
  }
}

void ZJsonObject::setEntry(const string &key, const string &value)
{
  setEntry(key.c_str(), value.c_str());
}

void ZJsonObject::setEntry(const char *key, const double *array, size_t n)
{
  if (!isValidKey(key)) {
    return;
  }

  setEntryWithoutKeyCheck(key, C_Json::makeArray(array, n), true);
}

void ZJsonObject::setEntry(const char *key, const int *array, size_t n)
{
  if (!isValidKey(key)) {
    return;
  }

  setEntryWithoutKeyCheck(key, C_Json::makeArray(array, n), true);
}

void ZJsonObject::setEntry(const char *key, bool v)
{
  if (!isValidKey(key)) {
    return;
  }

  setEntryWithoutKeyCheck(key, C_Json::makeBoolean(v));
}

void ZJsonObject::setEntry(const char *key, int64_t v)
{
  if (!isValidKey(key)) {
    return;
  }

  setEntryWithoutKeyCheck(key, C_Json::makeInteger(v), true);
}

void ZJsonObject::setEntry(const char *key, uint64_t v)
{
  if (!isValidKey(key)) {
    return;
  }

  setEntryWithoutKeyCheck(key, C_Json::makeInteger(v), true);
}

void ZJsonObject::setEntry(const char *key, int v)
{
  if (!isValidKey(key)) {
    return;
  }

  setEntryWithoutKeyCheck(key, C_Json::makeInteger(v), true);
}

void ZJsonObject::setEntry(const char *key, double v)
{
  if (!isValidKey(key)) {
    return;
  }

  setEntryWithoutKeyCheck(key, C_Json::makeNumber(v), true);
}

void ZJsonObject::setEntry(const char *key, ZJsonValue &value)
{
  if (!isValidKey(key)) {
    return;
  }

  setEntryWithoutKeyCheck(key, value.getValue());
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

std::string ZJsonObject::dumpString(int indent) const
{
  if (isEmpty()) {
    return "{}";
  }

  return ZJsonValue::dumpString(indent);
}
