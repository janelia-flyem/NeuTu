#include "zflyembodyannotation.h"

#include <iostream>
#include <sstream>
#include <algorithm>

#include "zjsonparser.h"
#include "zjsonobject.h"
#include "zstring.h"

const char *ZFlyEmBodyAnnotation::KEY_BODY_ID = "body ID";
const char *ZFlyEmBodyAnnotation::KEY_NAME = "name";
const char *ZFlyEmBodyAnnotation::KEY_TYPE = "class";
const char *ZFlyEmBodyAnnotation::KEY_COMMENT = "comment";
const char *ZFlyEmBodyAnnotation::KEY_STATUS = "status";
const char *ZFlyEmBodyAnnotation::KEY_USER = "user";
const char *ZFlyEmBodyAnnotation::KEY_NAMING_USER = "naming user";

ZFlyEmBodyAnnotation::ZFlyEmBodyAnnotation()
{
}

/*member dependent*/
void ZFlyEmBodyAnnotation::clear()
{
  m_bodyId = 0;
  m_status.clear();
  m_comment.clear();
  m_name.clear();
  m_type.clear();
  m_userName.clear();
  m_namingUser.clear();
}

void ZFlyEmBodyAnnotation::loadJsonString(const std::string &str)
{
  clear();

  ZJsonObject obj;
  obj.decodeString(str.c_str());

  loadJsonObject(obj);
}

/*member dependent*/
ZJsonObject ZFlyEmBodyAnnotation::toJsonObject() const
{
  ZJsonObject obj;
  if (m_bodyId > 0) {
    obj.setEntry(KEY_BODY_ID, m_bodyId);

    if (!m_name.empty()) {
      obj.setEntry(KEY_NAME, m_name);
    }

    if (!m_type.empty()) {
      obj.setEntry(KEY_TYPE, m_type);
    }

    if (!m_status.empty()) {
      obj.setEntry(KEY_STATUS, m_status);
    }

    if (!m_comment.empty()) {
      obj.setEntry(KEY_COMMENT, m_comment);
    }

    if (!m_userName.empty()) {
      obj.setEntry(KEY_USER, m_userName);
    }

    if (!m_namingUser.empty()) {
      obj.setEntry(KEY_NAMING_USER, m_namingUser);
    }
  }

  return obj;
}

/*member dependent*/
void ZFlyEmBodyAnnotation::loadJsonObject(const ZJsonObject &obj)
{
  if (obj.hasKey(KEY_BODY_ID) || obj.hasKey(KEY_STATUS) ||
      obj.hasKey(KEY_COMMENT) || obj.hasKey(KEY_NAME) ||
      obj.hasKey(KEY_TYPE) || obj.hasKey(KEY_NAMING_USER)) {
    if (obj.hasKey(KEY_BODY_ID)) {
      setBodyId(ZJsonParser::integerValue(obj[KEY_BODY_ID]));
    }

    if (obj.hasKey(KEY_STATUS)) {
      setStatus(ZJsonParser::stringValue(obj[KEY_STATUS]));
    }

    if (obj.hasKey(KEY_COMMENT)) {
      setComment(ZJsonParser::stringValue(obj[KEY_COMMENT]));
    }

    if (obj.hasKey(KEY_NAME)) {
      setName(ZJsonParser::stringValue(obj[KEY_NAME]));
    }

    if (obj.hasKey(KEY_TYPE)) {
      setType(ZJsonParser::stringValue(obj[KEY_TYPE]));
    }

    if (obj.hasKey(KEY_USER)) {
      setUser(ZJsonParser::stringValue(obj[KEY_USER]));
    }

    if (obj.hasKey(KEY_NAMING_USER)) {
      setNamingUser(ZJsonParser::stringValue(obj[KEY_NAMING_USER]));
    }
  } else {
    std::vector<std::string> keyList = obj.getAllKey();
    if (keyList.size() == 1) {
      uint64_t bodyId = ZString(keyList.front()).firstUint64();
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

/*member dependent*/
void ZFlyEmBodyAnnotation::print() const
{
  std::cout << "Body annotation:" << std::endl;
  std::cout << "  Body ID: " << m_bodyId << std::endl;
  std::cout << "  Type: " << m_type << std::endl;
  std::cout << "  Name: " << m_name << std::endl;
  std::cout << "  Status: " << m_status << std::endl;
  std::cout << "  Comment: " << m_comment << std::endl;
  std::cout << "  User: " << KEY_USER << std::endl;
  std::cout << "  Named User: " << KEY_NAMING_USER << std::endl;
}

/*member dependent*/
bool ZFlyEmBodyAnnotation::isEmpty() const
{
  return m_status.empty() && m_comment.empty() && m_name.empty() &&
      m_type.empty();
}

int ZFlyEmBodyAnnotation::GetStatusRank(const std::string &status)
{
  if (status.empty()) {
    return 9999;
  }

  std::string statusLowerCase = status;
  std::transform(statusLowerCase.begin(), statusLowerCase.end(),
                 statusLowerCase.begin(), ::tolower);

  if (statusLowerCase == "finalized") {
    return 0;
  }

  if (statusLowerCase == "traced") {
    return 10;
  }

  if (statusLowerCase == "traced in roi") {
    return 20;
  }

  if (statusLowerCase == "roughly traced") {
    return 30;
  }

  if (statusLowerCase == "prelim roughly traced") {
    return 40;
  }

  if (statusLowerCase == "partially traced") {
    return 50;
  }

  if (statusLowerCase == "hard to trace") {
    return 60;
  }

  if (statusLowerCase == "leaves") {
    return 70;
  }

  if (statusLowerCase == "not examined") {
    return 80;
  }

  if (statusLowerCase == "orphan hotknife") {
    return 90;
  }

  if (statusLowerCase == "orphan") {
    return 100;
  }

  return 999;
}

#if 0
int ZFlyEmBodyAnnotation::CompareStatus(
      const std::string &status1, const std::string &status2)
{
  if (GetStatusRank(status1))
}
#endif

void ZFlyEmBodyAnnotation::mergeAnnotation(
    const ZFlyEmBodyAnnotation &annotation)
{
  if (m_bodyId == 0) {
    m_bodyId = annotation.getBodyId();
  }

  if (GetStatusRank(m_status) > GetStatusRank(annotation.m_status)) {
    m_status = annotation.m_status;
  }

  if (m_comment.empty()) {
    m_comment = annotation.m_comment;
  }

  if (m_name.empty()) {
    m_name = annotation.m_name;
    m_namingUser = annotation.m_namingUser;
  }

  if (m_type.empty()) {
    m_type = annotation.m_type;
  }

  if (m_userName.empty()) {
    m_userName = annotation.m_userName;
  }
}

std::string ZFlyEmBodyAnnotation::toString() const
{
  std::ostringstream stream;

  if (isEmpty()) {
    stream << "[]";
  } else {
    stream << "[ ";
    if (!getName().empty()) {
      stream << getName() << " (" << getBodyId() << ")";
    } else {
      stream << getBodyId();
    }
    stream << ", " << getStatus() << " ]";
  }

  return stream.str();
}

bool ZFlyEmBodyAnnotation::isFinalized() const
{
  return (ZString(getStatus()).lower() == "finalized");
}
