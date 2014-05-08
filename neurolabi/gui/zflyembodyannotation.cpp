#include "zflyembodyannotation.h"

#include <iostream>
#include "zjsonparser.h"
#include "zjsonobject.h"

const char *ZFlyEmBodyAnnotation::m_bodyIdKey = "body ID";
const char *ZFlyEmBodyAnnotation::m_nameKey = "name";
const char *ZFlyEmBodyAnnotation::m_classKey = "class";
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
  m_class.clear();
}

void ZFlyEmBodyAnnotation::loadJsonString(const std::string &str)
{
  clear();

  ZJsonObject obj;
  obj.decodeString(str.c_str());
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

  if (obj.hasKey(m_classKey)) {
    setClass(ZJsonParser::stringValue(obj[m_classKey]));
  }
}

void ZFlyEmBodyAnnotation::print() const
{
  std::cout << "Body annotation:" << std::endl;
  std::cout << "  Body ID: " << m_bodyId << std::endl;
  std::cout << "  Class: " << m_class << std::endl;
  std::cout << "  Name: " << m_name << std::endl;
  std::cout << "  Status: " << m_status << std::endl;
  std::cout << "  Comment: " << m_comment << std::endl;
}
