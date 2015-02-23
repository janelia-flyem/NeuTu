#include "zflyembodyannotation.h"

#include <iostream>
#include "zjsonparser.h"
#include "zjsonobject.h"
#include "zstring.h"

const char *ZFlyEmBodyAnnotation::m_bodyIdKey = "body ID";
const char *ZFlyEmBodyAnnotation::m_nameKey = "name";
const char *ZFlyEmBodyAnnotation::m_typeKey = "class";
const char *ZFlyEmBodyAnnotation::m_commentKey = "comment";
const char *ZFlyEmBodyAnnotation::m_statusKey = "status";

ZFlyEmBodyAnnotation::ZFlyEmBodyAnnotation() : m_bodyId(-1)
{
}

void ZFlyEmBodyAnnotation::clear()
{
  m_bodyId = -1;
  m_status.clear();
  m_comment.clear();
  m_name.clear();
  m_type.clear();
}

void ZFlyEmBodyAnnotation::loadJsonString(const std::string &str)
{
  clear();

  ZJsonObject obj;
  obj.decodeString(str.c_str());

  loadJsonObject(obj);
}

void ZFlyEmBodyAnnotation::loadJsonObject(const ZJsonObject &obj)
{
  if (obj.hasKey(m_bodyIdKey) || obj.hasKey(m_statusKey) ||
      obj.hasKey(m_commentKey) || obj.hasKey(m_nameKey) ||
      obj.hasKey(m_typeKey)) {
    if (obj.hasKey(m_bodyIdKey)) {
      setBodyId(ZJsonParser::integerValue(obj[m_bodyIdKey]));
    }

    if (obj.hasKey(m_statusKey)) {
      setStatus(ZJsonParser::stringValue(obj[m_statusKey]));
    }

    if (obj.hasKey(m_commentKey)) {
      setComment(ZJsonParser::stringValue(obj[m_commentKey]));
    }

    if (obj.hasKey(m_nameKey)) {
      setName(ZJsonParser::stringValue(obj[m_nameKey]));
    }

    if (obj.hasKey(m_typeKey)) {
      setType(ZJsonParser::stringValue(obj[m_typeKey]));
    }
  } else {
    std::vector<std::string> keyList = obj.getAllKey();
    if (keyList.size() == 1) {
      int bodyId = ZString(keyList.front()).firstInteger();
      if (bodyId > 0) {
        setBodyId(bodyId);
        ZJsonObject annotationJson(
            const_cast<json_t*>(obj[keyList.front().c_str()]),
            ZJsonObject::SET_INCREASE_REF_COUNT);
        setType(ZJsonParser::stringValue(annotationJson["Type"]));
        setName(ZJsonParser::stringValue(annotationJson["Name"]));
      }
    }
  }
}

void ZFlyEmBodyAnnotation::print() const
{
  std::cout << "Body annotation:" << std::endl;
  std::cout << "  Body ID: " << m_bodyId << std::endl;
  std::cout << "  Type: " << m_type << std::endl;
  std::cout << "  Name: " << m_name << std::endl;
  std::cout << "  Status: " << m_status << std::endl;
  std::cout << "  Comment: " << m_comment << std::endl;
}
