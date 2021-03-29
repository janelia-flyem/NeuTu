#include "zflyembodyannotation.h"

#include <iostream>
#include <sstream>
#include <algorithm>

#include "zjsonparser.h"
#include "zjsonobject.h"
#include "zjsonobjectparser.h"
#include "zstring.h"

const char *ZFlyEmBodyAnnotation::KEY_BODY_ID = "body ID";
const char *ZFlyEmBodyAnnotation::KEY_NAME = "name";
const char *ZFlyEmBodyAnnotation::KEY_TYPE = "class";
const char *ZFlyEmBodyAnnotation::KEY_COMMENT = "comment";
const char *ZFlyEmBodyAnnotation::KEY_STATUS = "status";
const char *ZFlyEmBodyAnnotation::KEY_USER = "user";
const char *ZFlyEmBodyAnnotation::KEY_NAMING_USER = "naming user";
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
  m_instance.clear();
  m_majorInput.clear();
  m_majorOutput.clear();
  m_primaryNeurite.clear();
  m_location.clear();
  m_outOfBounds = false;
  m_crossMidline = false;
  m_neurotransmitter.clear();
  m_hemilineage.clear();
  m_synonym.clear();
  m_clonalUnit.clear();
  m_autoType.clear();
  m_property.clear();
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
/*
namespace {
template<typename T>
void process_annotation_key(
    const ZJsonObject &obj, const char *key, std::function<void(const T &)> f)
{
  if (obj.hasKey(key)) {
    f(ZJsonParser().getValue<T>(obj[key]));
  }
}
}
*/

void ZFlyEmBodyAnnotation::setBodyId(int64_t bodyId)
{
  if (bodyId < 0) {
    setBodyId(0);
  } else {
    setBodyId(uint64_t(bodyId));
  }
}

void ZFlyEmBodyAnnotation::setBodyId(int bodyId)
{
  setBodyId(int64_t(bodyId));
}

/*member dependent*/
void ZFlyEmBodyAnnotation::loadJsonObject(const ZJsonObject &obj)
{
  clear();

  std::string key = GetOldFormatKey(obj);
  if (!key.empty()) {
    uint64_t bodyId = ZString(key).firstUint64();
    if (bodyId > 0) {
      setBodyId(bodyId);
      ZJsonObject annotationJson(
          const_cast<json_t*>(obj[key.c_str()]),
          ZJsonObject::SET_INCREASE_REF_COUNT);
      setType(ZJsonParser::stringValue(annotationJson["Type"]));
      setName(ZJsonParser::stringValue(annotationJson["Name"]));
    }
  } else {
    ZJsonObjectParser objParser;

    if (obj.hasKey(KEY_BODY_ID)) {
      setBodyId(ZJsonParser::integerValue(obj[KEY_BODY_ID]));
    }

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

    if (obj.hasKey(KEY_TYPE)) {
      setType(ZJsonParser::stringValue(obj[KEY_TYPE]));
    }

    if (obj.hasKey(KEY_USER)) {
      setUser(ZJsonParser::stringValue(obj[KEY_USER]));
    }

    if (obj.hasKey(KEY_NAMING_USER)) {
      setNamingUser(ZJsonParser::stringValue(obj[KEY_NAMING_USER]));
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
    /*
    if (obj.hasKey(KEY_PRIMARY_NEURITE)) {
      setPrimaryNeurite(ZJsonParser::stringValue(obj[KEY_PRIMARY_NEURITE]));
    }
    */

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
    /*
    if (obj.hasKey(KEY_SYNONYM)) {
      setSynonym(ZJsonParser::stringValue(obj[KEY_SYNONYM]));
    }
    */

    if (obj.hasKey(KEY_CLONAL_UNIT)) {
      setClonalUnit(ZJsonParser::stringValue(obj[KEY_CLONAL_UNIT]));
    }

    if (obj.hasKey(KEY_AUTO_TYPE)) {
      setAutoType(ZJsonParser::stringValue(obj[KEY_AUTO_TYPE]));
    }

    /*
    process_annotation_key<std::string>(
          obj, KEY_MAJOR_OUTPUT,
          std::bind(&ZFlyEmBodyAnnotation::setMajorOutput, this, std::placeholders::_1));
          */
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

//  return getAutoName();
}

std::string ZFlyEmBodyAnnotation::getType() const
{
  return m_type;
//  if (!m_type.empty()) {
//    return m_type;
//  }

//  return getAutoType();
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

/*member dependent*/
void ZFlyEmBodyAnnotation::print() const
{
  std::cout << "Body annotation:" << std::endl;
  std::cout << "  Body ID: " << m_bodyId << std::endl;
  std::cout << "  Type: " << m_type << std::endl;
  std::cout << "  Name: " << getName() << std::endl;
  std::cout << "  Status: " << m_status << std::endl;
  std::cout << "  Propert: " << m_property << std::endl;
  std::cout << "  Comment: " << m_comment << std::endl;
  std::cout << "  User: " << m_userName << std::endl;
  std::cout << "  Named User: " << m_namingUser << std::endl;
  std::cout << "  Instance: " << m_instance << std::endl;
  std::cout << "  Major Input: " << m_majorInput << std::endl;
  std::cout << "  Major Output: " << m_majorOutput << std::endl;
  std::cout << "  Primary Neurite: " << m_primaryNeurite << std::endl;
  std::cout << "  Location: " << m_location << std::endl;
  std::cout << "  Out of Bounds: " << m_outOfBounds << std::endl;
  std::cout << "  Cross Midline: " << m_crossMidline << std::endl;
  std::cout << "  Neurotransmitter: " << m_neurotransmitter << std::endl;
  std::cout << "  Hemilineage" << m_hemilineage << std::endl;
  std::cout << "  Synonym: " << m_synonym << std::endl;
}

/*member dependent*/
bool ZFlyEmBodyAnnotation::isEmpty() const
{
  return m_status.empty() && m_comment.empty() &&
      m_type.empty() && getName().empty();
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
  if (m_bodyId == 0) {
    m_bodyId = annotation.getBodyId();
  }

  if (getStatusRank(m_status) > getStatusRank(annotation.m_status)) {
//    m_status = annotation.m_status;
    *this = annotation;
  } else if (getStatusRank(m_status) == getStatusRank(annotation.m_status)) {
    if (m_comment.empty()) {
      m_comment = annotation.m_comment;
    }

    if (m_name.empty()) {
      m_name = annotation.m_name;
      m_namingUser = annotation.m_namingUser;
    }

    if (m_instance.empty()) {
      m_instance = annotation.m_instance;
      m_namingUser = annotation.m_namingUser;
    }

    if (m_type.empty()) {
      m_type = annotation.m_type;
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

std::string ZFlyEmBodyAnnotation::toString() const
{
  std::ostringstream stream;

  if (isEmpty()) {
    stream << "[]";
  } else {
    stream << "[ ";
    if (!getName().empty()) {
      stream << getName();
    }

    if (!getType().empty()) {
      stream << ":" << getType();
    }

    stream << " (" << getBodyId() << ")";

    stream << ", " << getStatus() << " ]";
  }

  return stream.str();
}

bool ZFlyEmBodyAnnotation::isFinalized() const
{
  return (ZString(getStatus()).lower() == "finalized");
}

bool ZFlyEmBodyAnnotation::hasSameUserStatus(const ZFlyEmBodyAnnotation &annot) const
{
  return (m_userName == annot.m_userName) &&
      (m_namingUser == annot.m_namingUser);
}

//member dependent
bool ZFlyEmBodyAnnotation::operator ==(const ZFlyEmBodyAnnotation &annot) const
{
  return (m_bodyId == annot.m_bodyId) &&
      (m_status == annot.m_status) &&
      (m_comment == annot.m_comment) &&
      (m_name == annot.m_name) &&
      (m_type == annot.m_type) &&
//      (m_userName == annot.m_userName) &&
//      (m_namingUser == annot.m_namingUser) &&
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
