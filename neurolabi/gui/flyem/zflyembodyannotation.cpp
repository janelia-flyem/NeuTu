#include "zflyembodyannotation.h"

#include <iostream>
#include <sstream>
#include <algorithm>

#include "common/utilities.h"
#include "common/debug.h"

#include "zjsonparser.h"
#include "zjsonobject.h"
#include "zjsonobjectparser.h"
#include "zstring.h"

const char *ZFlyEmBodyAnnotation::KEY_BODY_ID = "bodyid";
const char *ZFlyEmBodyAnnotation::KEY_NAME = "name";
const char *ZFlyEmBodyAnnotation::KEY_CLASS = "class";
const char *ZFlyEmBodyAnnotation::KEY_TYPE = "type";
const char *ZFlyEmBodyAnnotation::KEY_HEMIBRAIN_TYPE = "hemibrain_type";
const char *ZFlyEmBodyAnnotation::KEY_COMMENT = "comment";
const char *ZFlyEmBodyAnnotation::KEY_DESCRIPTION = "description";
const char *ZFlyEmBodyAnnotation::KEY_STATUS = "status";
const char *ZFlyEmBodyAnnotation::KEY_USER = "user";
const char *ZFlyEmBodyAnnotation::KEY_INSTANCE = "instance";
const char *ZFlyEmBodyAnnotation::KEY_PROPERTY = "property";
const char *ZFlyEmBodyAnnotation::KEY_MAJOR_INPUT = "major input";
const char *ZFlyEmBodyAnnotation::KEY_MAJOR_OUTPUT = "major output";
const char *ZFlyEmBodyAnnotation::KEY_PRIMARY_NEURITE = "primary neurite";
const char *ZFlyEmBodyAnnotation::KEY_CELL_BODY_FIBER = "cell body fiber";
const char *ZFlyEmBodyAnnotation::KEY_LOCATION = "location";
const char *ZFlyEmBodyAnnotation::KEY_OUT_OF_BOUNDS = "out of bounds";
const char *ZFlyEmBodyAnnotation::KEY_CROSS_MIDLINE = "cross midline";
const char *ZFlyEmBodyAnnotation::KEY_NEURONTRANSMITTER = "neurotransmitter";
const char *ZFlyEmBodyAnnotation::KEY_HEMILINEAGE = "hemilineage";
const char *ZFlyEmBodyAnnotation::KEY_SYNONYM = "synonym";
const char *ZFlyEmBodyAnnotation::KEY_NOTES = "notes";
const char *ZFlyEmBodyAnnotation::KEY_CLONAL_UNIT = "clonal unit";
const char *ZFlyEmBodyAnnotation::KEY_AUTO_TYPE = "auto-type";
const char *ZFlyEmBodyAnnotation::KEY_TIMESTAMP = "timestamp";

ZFlyEmBodyAnnotation::ZFlyEmBodyAnnotation()
{
}

/*member dependent*/
void ZFlyEmBodyAnnotation::clear()
{
  *this = ZFlyEmBodyAnnotation();
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

  // strip off any fields ending with _user or _time; DVID will set these, and
  //    we should not post them
  for (std::string key: obj.getAllKey()) {
      if (key.length() > 5) {
          std::string end5 = key.substr(key.length() - 5);
          if (end5 == "_user" || end5 == "_time") {
              obj.removeKey(key.c_str());
          }
      }
  }

  if (!m_name.empty()) {
    obj.setEntry(KEY_NAME, m_name);
  }

  if (!m_class.empty()) {
    obj.setEntry(KEY_CLASS, m_class);
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

  obj.setNonEmptyEntry(KEY_INSTANCE, m_instance);
  obj.setNonEmptyEntry(KEY_MAJOR_INPUT, m_majorInput);
  obj.setNonEmptyEntry(KEY_MAJOR_OUTPUT, m_majorOutput);
  obj.setNonEmptyEntry(KEY_CELL_BODY_FIBER, m_primaryNeurite);
  obj.setNonEmptyEntry(KEY_LOCATION, m_location);
  obj.setTrueEntry(KEY_OUT_OF_BOUNDS, m_outOfBounds);
  obj.setTrueEntry(KEY_CROSS_MIDLINE, m_crossMidline);
  obj.setNonEmptyEntry(KEY_NEURONTRANSMITTER, m_neurotransmitter);
  obj.setNonEmptyEntry(KEY_HEMILINEAGE, m_hemilineage);
  obj.setNonEmptyEntry(KEY_NOTES, m_synonym);
  obj.setNonEmptyEntry(KEY_CLONAL_UNIT, m_clonalUnit);
  obj.setNonEmptyEntry(KEY_AUTO_TYPE, m_autoType);
  obj.setNonEmptyEntry(KEY_PROPERTY, m_property);

  if (m_timestamp > 0) {
    obj.setEntry(KEY_TIMESTAMP, m_timestamp);
  }

  return obj;
}

std::string ZFlyEmBodyAnnotation::GetOldFormatKey(const ZJsonObject &obj)
{
  std::vector<std::string> keyList = obj.getAllKey();
  if (keyList.size() == 1) {
    std::string &key = keyList.front();
    if (ZString(key).isAllDigit()) {
      return key;
    }
  }

  return "";
}

bool ZFlyEmBodyAnnotation::IsSameAnnotation(
    const ZJsonObject &obj1, const ZJsonObject &obj2)
{
  if (obj1.isEmpty() && obj2.isEmpty()) {
    return true;
  }


  int nkey1 = obj1.countKey();
  int nkey2 = obj2.countKey();

  if (nkey1 == nkey2) {
    return obj1.dumpJanssonString(JSON_INDENT(0) | JSON_SORT_KEYS) ==
        obj2.dumpJanssonString(JSON_INDENT(0) | JSON_SORT_KEYS);
  }

  return false;
}

bool ZFlyEmBodyAnnotation::IsEmptyAnnotation(const ZJsonObject &obj)
{
  return obj.all([&](const std::string &key, ZJsonValue value) {
    if (key == KEY_BODY_ID) {
      return true;
    }
#ifdef _DEBUG_0
    std::cout << "value: " << value.dumpString(0) << std::endl;
#endif
    return value.isEmpty();
  });
}

/*member dependent*/
void ZFlyEmBodyAnnotation::loadJsonObject(const ZJsonObject &obj)
{
  clear();

  std::string key = GetOldFormatKey(obj);
  if (!key.empty()) {
    uint64_t bodyId = ZString(key).firstUint64();
    if (bodyId > 0) {
      ZJsonObject annotationJson(
          const_cast<json_t*>(obj[key.c_str()]),
          ZJsonObject::SET_INCREASE_REF_COUNT);
      setClass(ZJsonParser::stringValue(annotationJson["Type"]));
      setName(ZJsonParser::stringValue(annotationJson["Name"]));
    }
  } else {
    ZJsonObjectParser objParser;

    if (obj.hasKey(KEY_STATUS)) {
      setStatus(ZJsonParser::stringValue(obj[KEY_STATUS]));
    }

    if (obj.hasKey(KEY_PROPERTY)) {
      setProperty(ZJsonParser::stringValue(obj[KEY_PROPERTY]));
    }

    if (obj.hasKey(KEY_COMMENT)) {
      setComment(ZJsonParser::stringValue(obj[KEY_COMMENT]));
    }

    if (obj.hasKey(KEY_NAME)) {
      setName(ZJsonParser::stringValue(obj[KEY_NAME]));
    }

    if (obj.hasKey(KEY_CLASS)) {
      setClass(ZJsonParser::stringValue(obj[KEY_CLASS]));
    }

    if (obj.hasKey(KEY_USER)) {
      setUser(ZJsonParser::stringValue(obj[KEY_USER]));
    }

    if (obj.hasKey(KEY_INSTANCE)) {
      setInstance(ZJsonParser::stringValue(obj[KEY_INSTANCE]));
    }

    if (obj.hasKey(KEY_MAJOR_INPUT)) {
      setMajorInput(ZJsonParser::stringValue(obj[KEY_MAJOR_INPUT]));
    }

    if (obj.hasKey(KEY_MAJOR_OUTPUT)) {
      setMajorOutput(ZJsonParser::stringValue(obj[KEY_MAJOR_OUTPUT]));
    }

    setPrimaryNeurite(
          objParser.GetValue(
            obj, std::vector<std::string>{KEY_CELL_BODY_FIBER, KEY_PRIMARY_NEURITE},
            std::string()));

    if (obj.hasKey(KEY_LOCATION)) {
      setLocation(ZJsonParser::stringValue(obj[KEY_LOCATION]));
    }

    if (obj.hasKey(KEY_OUT_OF_BOUNDS)) {
      setOutOfBounds(ZJsonParser::booleanValue(obj[KEY_OUT_OF_BOUNDS]));
    }

    if (obj.hasKey(KEY_CROSS_MIDLINE)) {
      setCrossMidline(ZJsonParser::booleanValue(obj[KEY_CROSS_MIDLINE]));
    }

    if (obj.hasKey(KEY_NEURONTRANSMITTER)) {
      setNeurotransmitter(ZJsonParser::stringValue(obj[KEY_NEURONTRANSMITTER]));
    }

    if (obj.hasKey(KEY_HEMILINEAGE)) {
      setHemilineage(ZJsonParser::stringValue(obj[KEY_HEMILINEAGE]));
    }

    setSynonym(objParser.GetValue(
                 obj, std::vector<std::string>{KEY_NOTES, KEY_SYNONYM}, std::string()));

    if (obj.hasKey(KEY_CLONAL_UNIT)) {
      setClonalUnit(ZJsonParser::stringValue(obj[KEY_CLONAL_UNIT]));
    }

    if (obj.hasKey(KEY_AUTO_TYPE)) {
      setAutoType(ZJsonParser::stringValue(obj[KEY_AUTO_TYPE]));
    }

    if (obj.hasKey(KEY_TIMESTAMP)) {
      m_timestamp = ZJsonParser::integerValue(obj[KEY_TIMESTAMP]);
    }
  }
}

std::string ZFlyEmBodyAnnotation::getMajorInput() const
{
  return m_majorInput;
}

std::string ZFlyEmBodyAnnotation::getMajorOutput() const
{
  return m_majorOutput;
}

std::string ZFlyEmBodyAnnotation::getPrimaryNeurite() const
{
  return m_primaryNeurite;
}

std::string ZFlyEmBodyAnnotation::getLocation() const
{
  return m_location;
}

bool ZFlyEmBodyAnnotation::getOutOfBounds() const
{
  return m_outOfBounds;
}

bool ZFlyEmBodyAnnotation::getCrossMidline() const
{
  return m_crossMidline;
}

std::string ZFlyEmBodyAnnotation::getNeurotransmitter() const
{
  return m_neurotransmitter;
}

std::string ZFlyEmBodyAnnotation::getHemilineage() const
{
  return m_hemilineage;
}

std::string ZFlyEmBodyAnnotation::getSynonym() const
{
  return m_synonym;
}

std::string ZFlyEmBodyAnnotation::getClonalUnit() const
{
  return m_clonalUnit;
}

std::string ZFlyEmBodyAnnotation::getProperty() const
{
  return m_property;
}

std::string ZFlyEmBodyAnnotation::getName() const
{
  if (!m_name.empty()) {
    return m_name;
  }

  return m_instance;
}

std::string ZFlyEmBodyAnnotation::getClass() const
{
  return m_class;
}

std::string ZFlyEmBodyAnnotation::getInferredType() const
{
  std::string type = m_majorInput + m_majorOutput;
  if (!m_primaryNeurite.empty()) {
    type += m_primaryNeurite;
  }

  return  type;
}

std::string ZFlyEmBodyAnnotation::getAutoType() const
{
  return m_autoType;
}

int64_t ZFlyEmBodyAnnotation::getTimestamp() const
{
  return m_timestamp;
}

void ZFlyEmBodyAnnotation::setMajorInput(const std::string &v)
{
  m_majorInput = v;
}

void ZFlyEmBodyAnnotation::setMajorOutput(const std::string &v)
{
  m_majorOutput = v;
}

void ZFlyEmBodyAnnotation::setPrimaryNeurite(const std::string &v)
{
  m_primaryNeurite = v;
}

void ZFlyEmBodyAnnotation::setLocation(const std::string &v)
{
  m_location = v;
}

void ZFlyEmBodyAnnotation::setOutOfBounds(bool v)
{
  m_outOfBounds = v;
}

void ZFlyEmBodyAnnotation::setCrossMidline(bool v)
{
  m_crossMidline = v;
}

void ZFlyEmBodyAnnotation::setNeurotransmitter(const std::string &v)
{
  m_neurotransmitter = v;
}

void ZFlyEmBodyAnnotation::setHemilineage(const std::string &v)
{
  m_hemilineage = v;
}

void ZFlyEmBodyAnnotation::setSynonym(const std::string &v)
{
  m_synonym = v;
}

void ZFlyEmBodyAnnotation::setClonalUnit(const std::string &v)
{
  m_clonalUnit = v;
}

void ZFlyEmBodyAnnotation::setAutoType(const std::string &v)
{
  m_autoType = v;
}

void ZFlyEmBodyAnnotation::setProperty(const std::string &v)
{
  m_property = v;
}

void ZFlyEmBodyAnnotation::updateTimestamp()
{
  m_timestamp = neutu::GetTimeStamp();
}

void ZFlyEmBodyAnnotation::UpdateTimeStamp(ZJsonObject &obj)
{
  obj.setEntry(KEY_TIMESTAMP, neutu::GetTimeStamp());
}

void ZFlyEmBodyAnnotation::print() const
{
  toJsonObject().print();
}

/*member dependent*/
bool ZFlyEmBodyAnnotation::isEmpty() const
{
  return m_status.empty() && m_comment.empty() &&
      m_class.empty() && getName().empty();
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

//Member dependent
void ZFlyEmBodyAnnotation::mergeAnnotation(const ZFlyEmBodyAnnotation &annotation,
    const std::function<int(const std::string&)>& getStatusRank)
{
  if (m_timestamp < annotation.m_timestamp) {
    m_timestamp = annotation.m_timestamp;
  }

  if (getStatusRank(m_status) > getStatusRank(annotation.m_status)) {
    *this = annotation;
  } else if (getStatusRank(m_status) == getStatusRank(annotation.m_status)) {
    if (m_comment.empty()) {
      m_comment = annotation.m_comment;
    }

    if (m_name.empty()) {
      m_name = annotation.m_name;
    }

    if (m_instance.empty()) {
      m_instance = annotation.m_instance;
    }

    if (m_class.empty()) {
      m_class = annotation.m_class;
    }

    if (m_userName.empty()) {
      m_userName = annotation.m_userName;
    }

    if (m_majorInput.empty()) {
      m_majorInput = annotation.m_majorInput;
    }

    if (m_majorOutput.empty()) {
      m_majorOutput = annotation.m_majorOutput;
    }

    if (m_primaryNeurite.empty()) {
      m_primaryNeurite = annotation.m_primaryNeurite;
    }

    if (m_location.empty()) {
      m_location = annotation.m_location;
    }

    if (m_property.empty()) {
      m_property = annotation.m_property;
    }

    m_outOfBounds = (m_outOfBounds || annotation.m_outOfBounds);
    m_crossMidline = (m_crossMidline || annotation.m_crossMidline);

    if (m_neurotransmitter.empty()) {
      m_neurotransmitter = annotation.m_neurotransmitter;
    }

    if (m_hemilineage.empty()) {
      m_hemilineage = annotation.m_hemilineage;
    }

    if (m_synonym.empty()) {
      m_synonym = annotation.m_synonym;
    }
  }

#if 0
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
#endif
}

std::string ZFlyEmBodyAnnotation::brief(uint64_t bodyId) const
{
  std::ostringstream stream;

  if (isEmpty()) {
    stream << "[]";
  } else {
    stream << "[ ";
    if (!getName().empty()) {
      stream << getName();
    }

    if (!getClass().empty()) {
      stream << ":" << getClass();
    }

    stream << " (" << bodyId << ")";

    stream << ", " << getStatus() << " ]";
  }

  return stream.str();
}

bool ZFlyEmBodyAnnotation::isFinalized() const
{
  return (ZString(getStatus()).lower() == "finalized");
}

bool ZFlyEmBodyAnnotation::IsFinalized(const ZJsonObject &obj)
{
  return (ZString(GetStatus(obj)).lower() == "finalized");
}

bool ZFlyEmBodyAnnotation::hasSameUserStatus(const ZFlyEmBodyAnnotation &annot) const
{
  return (m_userName == annot.m_userName);
}

//member dependent
bool ZFlyEmBodyAnnotation::operator ==(const ZFlyEmBodyAnnotation &annot) const
{
  return (m_bodyId == annot.m_bodyId) &&
      (m_status == annot.m_status) &&
      (m_comment == annot.m_comment) &&
      (m_name == annot.m_name) &&
      (m_class == annot.m_class) &&
      (m_instance == annot.m_instance) &&
      (m_property == annot.m_property) &&
      (m_majorInput == annot.m_majorInput) &&
      (m_majorOutput == annot.m_majorOutput) &&
      (m_primaryNeurite == annot.m_primaryNeurite) &&
      (m_location == annot.m_location) &&
      (m_outOfBounds == annot.m_outOfBounds) &&
      (m_crossMidline == annot.m_crossMidline) &&
      (m_neurotransmitter == annot.m_neurotransmitter) &&
      (m_hemilineage == annot.m_hemilineage) &&
      (m_synonym == annot.m_synonym);
}

std::string ZFlyEmBodyAnnotation::GetName(const ZJsonObject &obj)
{
  return ZJsonObjectParser::GetValue(
        obj, KEY_NAME, ZJsonObjectParser::GetValue(obj, KEY_INSTANCE, ""));
}

std::string ZFlyEmBodyAnnotation::GetStatus(const ZJsonObject &obj)
{
  return ZJsonObjectParser::GetValue(obj, KEY_STATUS, "");
}

std::string ZFlyEmBodyAnnotation::GetType(const ZFlyEmBodyAnnotation &obj)
{
  return obj.getType();
}

std::string ZFlyEmBodyAnnotation::GetClass(const ZJsonObject &obj)
{
  return ZJsonObjectParser::GetValue(obj, KEY_CLASS, "");
}

std::string ZFlyEmBodyAnnotation::GetType(const ZJsonObject &obj)
{
  return ZJsonObjectParser::GetValue(obj, KEY_TYPE, "");
}

void ZFlyEmBodyAnnotation::SetStatus(ZJsonObject &obj, const std::string &status)
{
  obj.setEntry(KEY_STATUS, status);
}

std::string ZFlyEmBodyAnnotation::GetUser(const ZJsonObject &obj)
{
  return ZJsonObjectParser::GetValue(obj, KEY_USER, "");
}

void ZFlyEmBodyAnnotation::SetUser(ZJsonObject &obj, const std::string &user)
{
  obj.setEntry(KEY_USER, user);
}

std::string ZFlyEmBodyAnnotation::GetName(const ZFlyEmBodyAnnotation &obj)
{
  return obj.getName();
}

std::string ZFlyEmBodyAnnotation::GetStatus(const ZFlyEmBodyAnnotation &obj)
{
  return obj.getStatus();
}

std::string ZFlyEmBodyAnnotation::GetClass(const ZFlyEmBodyAnnotation &obj)
{
  return obj.getClass();
}

ZJsonObject ZFlyEmBodyAnnotation::MergeAnnotation(
    const ZJsonObject &target, const ZJsonObject &source,
    const std::function<int (const std::string &)> &getStatusRank)
{
  ZJsonObject result;
  std::string targetStatus = ZJsonObjectParser::GetValue(
        target, "status", "");
  std::string sourceStatus = ZJsonObjectParser::GetValue(
        source, "status", "");
  if (getStatusRank(targetStatus) == getStatusRank(sourceStatus)) {
    result = target.clone();
    bool passingNamingUser = false;
    bool passingStatusUser = false;
    // Move source to empty fields of target
    source.forEachValue([&](const std::string &key, ZJsonValue value) {
        // check for _user and _time fields; don't move them
        if (key.length() > 5) {
            std::string end5 = key.substr(key.length() - 5);
            if (end5 == "_user" || end5 == "_time") {
                return;
            }
        }

        if (value.isBoolean()) {
        result.setEntry(
              key, ZJsonObjectParser::GetValue(target, key, false) ||
              value.toBoolean());
      } else {
        std::string stringValue = value.toString();
        if (ZJsonObjectParser::GetValue(target, key, "").empty() &&
            !stringValue.empty()) {
          result.setEntry(key, stringValue);
        }
      }
    });
    int64_t sourceTimeStamp = ZJsonObjectParser::GetValue(
          source, ZFlyEmBodyAnnotation::KEY_TIMESTAMP, 0ll);
    int64_t targetTimeStamp = ZJsonObjectParser::GetValue(
          result, ZFlyEmBodyAnnotation::KEY_TIMESTAMP, 0ll);
    if (sourceTimeStamp > targetTimeStamp) {
      result.setEntry(ZFlyEmBodyAnnotation::KEY_TIMESTAMP, sourceTimeStamp);
    }
  } else if (getStatusRank(targetStatus) > getStatusRank(sourceStatus)) { //source has higher priority
    HLDEBUG("annotate body") << "Use source annotation directly." << std::endl;
    result = source.clone();
  } else { //target has higher priority
    HLDEBUG("annotate body") << "Use target annotation directly." << std::endl;
    result = target.clone();
  }

  return result;
}

std::string ZFlyEmBodyAnnotation::GetComment(const ZJsonObject &obj)
{
  return ZJsonObjectParser::GetValue(
        obj, KEY_DESCRIPTION,
        ZJsonObjectParser::GetValue(obj, KEY_COMMENT, ""));
}

void ZFlyEmBodyAnnotation::SetComment(
    ZJsonObject &obj, const std::string &comment, bool usingDescription)
{
  if (comment.empty()) {
    obj.removeKey(KEY_COMMENT);
    obj.removeKey(KEY_DESCRIPTION);
  } else {
    if (usingDescription) {
      obj.removeKey(KEY_COMMENT);
      obj.setEntry(KEY_DESCRIPTION, comment);
    } else {
      obj.removeKey(KEY_DESCRIPTION);
      obj.setEntry(KEY_COMMENT, comment);
    }
  }
}

void ZFlyEmBodyAnnotation::UpdateUserFields(
    ZJsonObject &obj, const std::string &user, const ZJsonObject &oldObj)
{
  std::string newUser;
  if (ZFlyEmBodyAnnotation::GetUser(obj).empty()) {
    newUser = ZFlyEmBodyAnnotation::GetUser(oldObj);
    if (newUser.empty()) {
      newUser = user;
    }
  }

  if (!newUser.empty()) {
    ZFlyEmBodyAnnotation::SetUser(obj, newUser);
  }
}

std::string ZFlyEmBodyAnnotation::Brief(uint64_t bodyId, const ZJsonObject &obj)
{
  std::ostringstream stream;

  if (obj.isEmpty()) {
    stream << "[]";
  } else {
    stream << "[ ";
    if (!GetName(obj).empty()) {
      stream << GetName(obj);
    }

    if (!GetType(obj).empty()) {
      stream << ":" << GetType(obj);
    }

    if (!GetClass(obj).empty()) {
      stream << ":" << GetClass(obj);
    }

    stream << " (" << bodyId << ")";

    stream << ", " << GetStatus(obj) << " ]";
  }

  return stream.str();
}

std::string ZFlyEmBodyAnnotation::Brief(
    uint64_t bodyId, const ZFlyEmBodyAnnotation &obj)
{
  return obj.brief(bodyId);
}

