#include "zdvidtarget.h"
#include "zstring.h"
#include "zerror.h"
#include "zjsonparser.h"
#include "zdviddata.h"

const char* ZDvidTarget::m_addressKey = "address";
const char* ZDvidTarget::m_portKey = "port";
const char* ZDvidTarget::m_uuidKey = "uuid";
const char* ZDvidTarget::m_commentKey = "comment";
const char* ZDvidTarget::m_nameKey = "name";
const char* ZDvidTarget::m_localKey = "local";
const char* ZDvidTarget::m_debugKey = "debug";
const char* ZDvidTarget::m_bgValueKey = "background";
const char* ZDvidTarget::m_bodyLabelNameKey = "body_label";
const char* ZDvidTarget::m_labelBlockNameKey = "label_block";
const char* ZDvidTarget::m_grayScaleNameKey = "gray_scale";
const char* ZDvidTarget::m_multiscale2dNameKey = "multires_tile";

ZDvidTarget::ZDvidTarget() : m_port(-1), m_bgValue(255)
{
}

ZDvidTarget::ZDvidTarget(
    const std::string &address, const std::string &uuid, int port)
{
  set(address, uuid, port);
}

std::string ZDvidTarget::getSourceString(bool withHttpPrefix) const
{
  std::string source;

  if (!getAddress().empty()) {
    source = getAddress() + ":" + ZString::num2str(getPort()) + ":" + getUuid();
    if (withHttpPrefix) {
      source = "http:" + source;
    }
  }

  if (!m_bodyLabelName.empty()) {
    source += ":" + m_bodyLabelName;
  }

  return source;
}

void ZDvidTarget::set(
    const std::string &address, const std::string &uuid, int port)
{
  setServer(address);
  setUuid(uuid);
  setPort(port);
}

void ZDvidTarget::clear()
{
  set("", "", -1);
  m_name = "";
  m_comment = "";
  m_localFolder = "";
}

void ZDvidTarget::setServer(const std::string &address)
{
  if (ZString(address).startsWith("http://")) {
    m_address = address.substr(7);
  } else if (ZString(address).startsWith("//")) {
    m_address = address.substr(2);
  } else {
    m_address = address;
  }
}

void ZDvidTarget::setUuid(const std::string &uuid)
{
  m_uuid = uuid;
}

void ZDvidTarget::setPort(int port)
{
  m_port = port;
}

void ZDvidTarget::setFromUrl(const std::string &url)
{
  ZString zurl(url);
  if (zurl.startsWith("http:")) {
    zurl.replace("http://", "");
  }

  std::vector<std::string> tokens = zurl.tokenize('/');
  ZString addressToken = tokens[0];
  std::vector<std::string> tokens2 = addressToken.tokenize(':');
  int port = -1;
  if (tokens2.size() > 1) {
    if (!tokens2[1].empty()) {
      port = ZString::firstInteger(tokens2[1]);
      if (tokens2[1][0] == '-') {
        port = -port;
      }
    }
  }
  std::string uuid;
  if (tokens.size() > 3) {
    if (tokens[2] == "node") {
      uuid = tokens[3];
    }
  }
  set(tokens2[0], uuid, port);
}

void ZDvidTarget::setFromSourceString(const std::string &sourceString)
{
  set("", "", -1);

  std::vector<std::string> tokens = ZString(sourceString).tokenize(':');

  if (tokens.size() < 4 || tokens[0] != "http") {
    RECORD_WARNING_UNCOND("Invalid source string");
  } else {
    int port = -1;
    if (!tokens[2].empty()) {
      port = ZString::firstInteger(tokens[2]);
      if (tokens[2][0] == '-') {
        port = -port;
      }
    }
    set(tokens[1], tokens[3], port);
    if (tokens.size() >= 5) {
      setBodyLabelName(tokens[4]);
    }
  }
}

bool ZDvidTarget::hasPort() const
{
  return getPort() >= 0;
}

bool ZDvidTarget::isValid() const
{
  return !getAddress().empty() && !getUuid().empty();
}

std::string ZDvidTarget::getAddressWithPort() const
{
  std::string address;

  if (!getAddress().empty()) {
    address = getAddress();
    if (hasPort()) {
      address += ":" + ZString::num2str(getPort());
    }
  }

  return address;
}

void ZDvidTarget::print() const
{
  std::cout << getSourceString() << std::endl;
}

std::string ZDvidTarget::getBodyPath(int bodyId) const
{
  return getSourceString() + ":" + ZString::num2str(bodyId);
}

ZJsonObject ZDvidTarget::toJsonObject() const
{
  ZJsonObject obj;
  if (m_port >= 0) {
    obj.setEntry(m_portKey, m_port);
  }

  obj.setEntry(m_addressKey, m_address);
  obj.setEntry(m_uuidKey, m_uuid);
  obj.setEntry(m_commentKey, m_comment);
  obj.setEntry(m_nameKey, m_name);
  obj.setEntry(m_localKey, m_localFolder);
  obj.setEntry(m_bgValueKey, m_bgValue);
  obj.setEntry(m_bodyLabelNameKey, m_bodyLabelName);
  obj.setEntry(m_labelBlockNameKey, m_labelBlockName);
  obj.setEntry(m_grayScaleNameKey, m_grayScaleName);
  obj.setEntry(m_multiscale2dNameKey, m_multiscale2dName);

  return obj;
}

void ZDvidTarget::loadJsonObject(const ZJsonObject &obj)
{
  clear();

  bool isValidJson = true;

  if (obj.hasKey(m_debugKey)) {
#ifndef _DEBUG_
    isValidJson = !ZJsonParser::booleanValue(obj[m_debugKey]);
#endif
  }

  if (isValidJson) {
    setServer(ZJsonParser::stringValue(obj[m_addressKey]));
    if (obj.hasKey(m_portKey)) {
      setPort(ZJsonParser::integerValue(obj[m_portKey]));
    } else {
      setPort(-1);
    }
    setUuid(ZJsonParser::stringValue(obj[m_uuidKey]));
    m_comment = ZJsonParser::stringValue(obj[m_commentKey]);
    m_name = ZJsonParser::stringValue(obj[m_nameKey]);
    m_localFolder = ZJsonParser::stringValue(obj[m_localKey]);
    if (obj.hasKey(m_bgValueKey)) {
      m_bgValue = ZJsonParser::integerValue(obj[m_bgValueKey]);
    }
    if (obj.hasKey(m_bodyLabelNameKey)) {
      setBodyLabelName(ZJsonParser::stringValue(obj[m_bodyLabelNameKey]));
    }
    if (obj.hasKey(m_labelBlockNameKey)) {
      setLabelBlockName(ZJsonParser::stringValue(obj[m_labelBlockNameKey]));
    }
    if (obj.hasKey(m_grayScaleNameKey)) {
      setGrayScaleName(ZJsonParser::stringValue(obj[m_grayScaleNameKey]));
    }
    if (obj.hasKey(m_multiscale2dNameKey)) {
      setMultiscale2dName(ZJsonParser::stringValue(obj[m_multiscale2dNameKey]));
    }
  }
}

std::string ZDvidTarget::getUrl() const
{
  ZString url = "http://" + m_address;
  if (m_port >= 0) {
    url += ":";
    url.appendNumber(m_port);
  }
  url += "/api/node/" + m_uuid;

  return url;
}

std::string ZDvidTarget::getLocalLowResGrayScalePath(
    int xintv, int yintv, int zintv) const
{
  if (getLocalFolder().empty()) {
    return "";
  }

  ZString path = getLocalFolder() + "/grayscale/";
  path.appendNumber(xintv);
  path += "_";
  path.appendNumber(yintv);
  path += "_";
  path.appendNumber(zintv);

  return path;
}


std::string ZDvidTarget::getLocalLowResGrayScalePath(
    int xintv, int yintv, int zintv, int z) const
{
  if (getLocalFolder().empty()) {
    return "";
  }

  const int padding = 6;

  ZString path = getLocalLowResGrayScalePath(xintv, yintv, zintv);
  path += "/";
  path.appendNumber(z, padding);
  path += ".tif";

  return path;
}

std::string ZDvidTarget::getBodyLabelName() const
{
  if (m_bodyLabelName.empty()) {
    return ZDvidData::getName(ZDvidData::ROLE_BODY_LABEL);
  }

  return m_bodyLabelName;
}

std::string ZDvidTarget::getLabelBlockName() const
{
  if (m_labelBlockName.empty()) {
    return ZDvidData::getName(ZDvidData::ROLE_LABEL_BLOCK);
  }

  return m_labelBlockName;
}

void ZDvidTarget::setLabelBlockName(const std::string &name)
{
  m_labelBlockName = name;
}

std::string ZDvidTarget::getMultiscale2dName() const
{
  if (m_multiscale2dName.empty()) {
    return ZDvidData::getName(ZDvidData::ROLE_MULTISCALE_2D);
  }

  return m_multiscale2dName;
}

std::string ZDvidTarget::getGrayScaleName() const
{
  if (m_grayScaleName.empty()) {
    return ZDvidData::getName(ZDvidData::ROLE_GRAY_SCALE);
  }

  return m_grayScaleName;
}

void ZDvidTarget::setGrayScaleName(const std::string &name)
{
  m_grayScaleName = name;
}

void ZDvidTarget::setBodyLabelName(const std::string &name)
{
  m_bodyLabelName = name;
}

void ZDvidTarget::setMultiscale2dName(const std::string &name)
{
  m_multiscale2dName = name;
}

std::string ZDvidTarget::getName(ZDvidData::ERole role) const
{
  std::string name;

  switch (role) {
  case ZDvidData::ROLE_MULTISCALE_2D:
    name =  m_multiscale2dName;
    break;
  case ZDvidData::ROLE_BODY_LABEL:
    name = m_bodyLabelName;
    break;
  case ZDvidData::ROLE_LABEL_BLOCK:
    name = m_labelBlockName;
    break;
  case ZDvidData::ROLE_GRAY_SCALE:
    name = m_grayScaleName;
    break;
  default:
    break;
  }

  if (name.empty()) {
    name = ZDvidData::getName(role);
  }

  return name;
}
