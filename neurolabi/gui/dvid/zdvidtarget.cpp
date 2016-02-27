#include "zdvidtarget.h"
#include "zstring.h"
#include "zerror.h"
#include "zjsonparser.h"
#include "zdviddata.h"
#if _QT_APPLICATION_
#include <QtDebug>
#include "dvid/zdvidbufferreader.h"
#endif
#include "neutubeconfig.h"

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
const char* ZDvidTarget::m_synapseNameKey = "synapse";
const char* ZDvidTarget::m_userNameKey = "user_name";
const char* ZDvidTarget::m_supervisorKey = "supervised";
const char* ZDvidTarget::m_supervisorServerKey = "librarian";
const char* ZDvidTarget::m_roiNameKey = "roi";

ZDvidTarget::ZDvidTarget() : m_port(-1), m_isSupervised(true), m_bgValue(255),
  m_isEditable(true)
{
}

ZDvidTarget::ZDvidTarget(
    const std::string &address, const std::string &uuid, int port) :
  m_isSupervised(true), m_bgValue(255), m_isEditable(true)
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
  m_bodyLabelName = "";
  m_labelBlockName = "";
  m_multiscale2dName = "";
  m_grayScaleName = "";
//  m_roiName = "";
  m_synapseName = "";
  m_roiList.clear();
  m_userList.clear();
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

#if defined(_FLYEM_)
  m_address = GET_FLYEM_CONFIG.mapAddress(m_address);
#endif
}

void ZDvidTarget::setUuid(const std::string &uuid)
{
  if (ZString(uuid).startsWith("ref:")) {
#if _QT_APPLICATION_
    std::string uuidLink = uuid.substr(4);
    ZDvidBufferReader reader;
    reader.read(uuidLink.c_str());
    m_uuid = reader.getBuffer().constData();
#else
    m_uuid = "";
    std::cout << "Unsupported uuid ref in non-GUI application. No uuid is set."
              << std::endl;
#endif
  } else {
    m_uuid = uuid;
  }
}

void ZDvidTarget::setPort(int port)
{
  m_port = port;
}

void ZDvidTarget::setFromUrl(const std::string &url)
{
  if (url.empty()) {
    clear();
    return;
  }

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
#if defined(_QT_APPLICATION_)
    qWarning() << "Invalid source string for dvid target";
#else
    RECORD_WARNING_UNCOND("Invalid source string");
#endif
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

std::string ZDvidTarget::getBodyPath(uint64_t bodyId) const
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
  ZJsonArray jsonArray;
  for (std::vector<std::string>::const_iterator iter = m_roiList.begin();
       iter != m_roiList.end(); ++iter) {
    jsonArray.append(*iter);
  }
  obj.setEntry(m_roiNameKey, jsonArray);

  obj.setEntry(m_multiscale2dNameKey, m_multiscale2dName);
  obj.setEntry(m_synapseNameKey, m_synapseName);

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
    if (obj.hasKey(m_roiNameKey)) {
      ZJsonArray jsonArray(obj.value(m_roiNameKey));
      for (size_t i = 0; i < jsonArray.size(); ++i) {
        addRoiName(ZJsonParser::stringValue(jsonArray.getData(), i));
      }
//      setRoiName(ZJsonParser::stringValue(obj[m_roiNameKey]));
    }
    if (obj.hasKey(m_synapseNameKey)) {
      setSynapseName(ZJsonParser::stringValue(obj[m_synapseNameKey]));
    }

    if (obj.hasKey(m_userNameKey)) {
      ZJsonValue value = obj.value(m_userNameKey);
      if (value.isString()) {
        m_userList.insert(ZJsonParser::stringValue(value.getData()));
      } else if (value.isArray()) {
        ZJsonArray nameArray(value);
        for (size_t i = 0; i < nameArray.size(); ++i) {
          m_userList.insert(ZJsonParser::stringValue(nameArray.at(i)));
        }
      }
    }
    if (obj.hasKey(m_supervisorKey)) {
      ZJsonValue value = obj.value(m_supervisorKey);
      if (value.isBoolean()) {
        m_isSupervised= ZJsonParser::booleanValue(value.getData());
      }
    }
    if (obj.hasKey(m_supervisorServerKey)) {
      m_supervisorServer = ZJsonParser::stringValue(obj[m_supervisorServerKey]);
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
    return ZDvidData::GetName(ZDvidData::ROLE_BODY_LABEL);
  }

  return m_bodyLabelName;
}

std::string ZDvidTarget::getLabelBlockName() const
{
  if (m_labelBlockName.empty()) {
    return ZDvidData::GetName(ZDvidData::ROLE_LABEL_BLOCK);
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
    return ZDvidData::GetName(ZDvidData::ROLE_MULTISCALE_2D);
  }

  return m_multiscale2dName;
}

std::string ZDvidTarget::getGrayScaleName() const
{
  if (m_grayScaleName.empty()) {
    return ZDvidData::GetName(ZDvidData::ROLE_GRAY_SCALE);
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

std::string ZDvidTarget::getRoiName(size_t index) const
{
  return m_roiList[index];
}

void ZDvidTarget::addRoiName(const std::string &name)
{
  m_roiList.push_back(name);
//  m_roiName = name;
}

std::string ZDvidTarget::getSynapseName() const
{
  if (m_synapseName.empty()) {
    return ZDvidData::GetName(ZDvidData::ROLE_SYNAPSE);
  }

  return m_synapseName;
}

std::string ZDvidTarget::getBookmarkName() const
{
  return ZDvidData::GetName(ZDvidData::ROLE_BOOKMARK);
}

std::string ZDvidTarget::getBookmarkKeyName() const
{
  return ZDvidData::GetName(ZDvidData::ROLE_BOOKMARK_KEY);
}

std::string ZDvidTarget::getSkeletonName() const
{
  return ZDvidData::GetName(ZDvidData::ROLE_SKELETON, ZDvidData::ROLE_BODY_LABEL,
                            getBodyLabelName());
}

std::string ZDvidTarget::getThumbnailName() const
{
  return ZDvidData::GetName(ZDvidData::ROLE_THUMBNAIL, ZDvidData::ROLE_BODY_LABEL,
                            getBodyLabelName());
}

std::string ZDvidTarget::getTodoListName() const
{
  return ZDvidData::GetName(ZDvidData::ROLE_TODO_LIST,
                            ZDvidData::ROLE_BODY_LABEL,
                            getBodyLabelName());
}

void ZDvidTarget::setSynapseName(const std::string &name)
{
  m_synapseName = name;
}

/*
void ZDvidTarget::setUserName(const std::string &name)
{
  m_userName = name;
}
*/

const std::set<std::string>& ZDvidTarget::getUserNameSet() const
{
  return m_userList;
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
    name = ZDvidData::GetName(role);
  }

  return name;
}

bool ZDvidTarget::isDvidTarget(const std::string &source)
{
  return ZString(source).startsWith("http:");
}
