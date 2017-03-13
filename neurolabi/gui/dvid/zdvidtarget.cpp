#include "zdvidtarget.h"
#include "zstring.h"
#include "zerror.h"
#include "zjsonparser.h"
#include "zjsonobject.h"
#include "zdviddata.h"
#if _QT_APPLICATION_
#include <QtDebug>
#include "QsLog.h"
#include "dvid/zdvidbufferreader.h"
#endif
#include "neutubeconfig.h"

const char* ZDvidTarget::m_commentKey = "comment";
const char* ZDvidTarget::m_nameKey = "name";
const char* ZDvidTarget::m_localKey = "local";
const char* ZDvidTarget::m_debugKey = "debug";
const char* ZDvidTarget::m_bgValueKey = "background";
const char* ZDvidTarget::m_bodyLabelNameKey = "body_label";
const char* ZDvidTarget::m_labelBlockNameKey = "label_block";
const char* ZDvidTarget::m_grayScaleNameKey = "gray_scale";
const char* ZDvidTarget::m_multiscale2dNameKey = "multires_tile";
const char* ZDvidTarget::m_tileConfigKey = "multires_tile_config";
const char* ZDvidTarget::m_synapseNameKey = "synapse";
const char* ZDvidTarget::m_userNameKey = "user_name";
const char* ZDvidTarget::m_supervisorKey = "supervised";
const char* ZDvidTarget::m_supervisorServerKey = "librarian";
const char* ZDvidTarget::m_roiListKey = "roi_list";
const char* ZDvidTarget::m_roiNameKey = "roi";
const char* ZDvidTarget::m_maxLabelZoomKey = "label_max_zoom";
const char* ZDvidTarget::m_synapseLabelszKey = "labelsz";
const char* ZDvidTarget::m_todoListNameKey = "todo";
const char* ZDvidTarget::m_defaultSettingKey = "default";
const char* ZDvidTarget::m_sourceConfigKey = "@source";

ZDvidTarget::ZDvidTarget()
{
  init();
}

ZDvidTarget::ZDvidTarget(
    const std::string &address, const std::string &uuid, int port)
{
  init();

  set(address, uuid, port);
}

void ZDvidTarget::init()
{
  m_isSupervised = true;
  m_bgValue = 255;
  m_isEditable = true;
  m_readOnly = false;
  m_maxLabelZoom = 0;
  m_maxGrayscaleZoom = 0;
  m_usingMultresBodyLabel = true;
  m_usingDefaultSetting = false;

  setDefaultMultiscale2dName();
//  m_multiscale2dName = ZDvidData::GetName(ZDvidData::ROLE_MULTISCALE_2D);
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
  m_node.set(address, uuid, port);
}

void ZDvidTarget::clear()
{
  m_node.clear();
  init();
  m_name = "";
  m_comment = "";
  m_localFolder = "";
  m_bodyLabelName = "";
  m_labelBlockName = "";
//  m_multiscale2dName = "";

  m_grayScaleName = "";
  m_synapseLabelszName = "";
//  m_roiName = "";
  m_synapseName = "";
  m_roiName = "";
  m_todoListName = "";
  m_roiList.clear();
  m_userList.clear();
  m_supervisorServer.clear();
  m_tileConfig.clear();
  m_sourceConfig.clear();
//  m_tileJson = ZJsonArray();
}

void ZDvidTarget::setServer(const std::string &address)
{
  m_node.setServer(address);
}

void ZDvidTarget::setUuid(const std::string &uuid)
{
  m_node.setUuid(uuid);
}

void ZDvidTarget::setPort(int port)
{
  m_node.setPort(port);
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
    if (tokens[1] == "api" && tokens[2] == "node") {
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
    LWARN() << "Invalid source string for dvid target:" << sourceString.c_str();
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
  ZJsonObject obj = m_node.toJsonObject();
  obj.setEntry(m_commentKey, m_comment);
  obj.setEntry(m_nameKey, m_name);
  obj.setEntry(m_localKey, m_localFolder);
  obj.setEntry(m_bgValueKey, m_bgValue);
  obj.setEntry(m_bodyLabelNameKey, m_bodyLabelName);
  obj.setEntry(m_labelBlockNameKey, m_labelBlockName);
  obj.setEntry(m_maxLabelZoomKey, m_maxLabelZoom);
  obj.setEntry(m_grayScaleNameKey, m_grayScaleName);
  obj.setEntry(m_synapseLabelszKey, m_synapseLabelszName);
  obj.setEntry(m_roiNameKey, m_roiName);
  if (!m_todoListName.empty()) {
    obj.setEntry(m_todoListNameKey, m_todoListName);
  }
  ZJsonArray jsonArray;
  for (std::vector<std::string>::const_iterator iter = m_roiList.begin();
       iter != m_roiList.end(); ++iter) {
    jsonArray.append(*iter);
  }
  obj.setEntry(m_roiListKey, jsonArray);

  obj.setEntry(m_multiscale2dNameKey, m_multiscale2dName);
  if (!m_tileConfig.isEmpty()) {
    obj.setEntry(m_tileConfigKey, const_cast<ZJsonObject&>(m_tileConfig));
  }

  if (!m_sourceConfig.isEmpty()) {
    obj.setEntry(m_sourceConfigKey, const_cast<ZJsonObject&>(m_sourceConfig));
  }

  obj.setEntry(m_synapseNameKey, m_synapseName);
  obj.setEntry(m_supervisorKey, m_isSupervised);
  obj.setEntry(m_supervisorServerKey, m_supervisorServer);
  obj.setEntry(m_defaultSettingKey, usingDefaultDataSetting());

  return obj;
}

std::string ZDvidTarget::getSynapseLabelszName() const
{
  if (!m_synapseLabelszName.empty()) {
    return m_synapseLabelszName;
  }

  if (getSynapseName().empty()) {
    return "";
  }

  return ZDvidData::GetName(ZDvidData::ROLE_LABELSZ, ZDvidData::ROLE_SYNAPSE,
                            getSynapseName());
}

void ZDvidTarget::setSynapseLabelszName(const std::string &name)
{
  m_synapseLabelszName = name;
}

void ZDvidTarget::loadDvidDataSetting(const ZJsonObject &obj)
{
  if (obj.hasKey("segmentation")) {
    setLabelBlockName(ZJsonParser::stringValue(obj["segmentation"]));
  }
  if (obj.hasKey("bodies")) {
    setBodyLabelName(ZJsonParser::stringValue(obj["bodies"]));
  }
  if (obj.hasKey("synapses")) {
    setSynapseName(ZJsonParser::stringValue(obj["synapses"]));
  }
  if (obj.hasKey("grayscale")) {
    setGrayScaleName(ZJsonParser::stringValue(obj["grayscale"]));
  }

  if (obj.hasKey("todos")) {
    setTodoListName(ZJsonParser::stringValue(obj["todos"]));
  }
}

ZJsonObject ZDvidTarget::toDvidDataSetting() const
{
  ZJsonObject obj;

  obj.setEntry("segmentation", getLabelBlockName());
  obj.setEntry("synapses", getSynapseName());
  obj.setEntry("bodies", getBodyLabelName());
  obj.setEntry("skeletons", getSkeletonName());
  obj.setEntry("grayscale", getGrayScaleName());
  obj.setEntry("body_annotations", getBodyAnnotationName());
  obj.setEntry("todos", getTodoListName());
  obj.setEntry("sequencer_info", getSynapseLabelszName());
  obj.setEntry("bookmarks", getBookmarkName());

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
    m_node.loadJsonObject(obj);
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
      setMultiscale2dName(
            ZJsonParser::stringValue(obj[m_multiscale2dNameKey]));
    }
    if (obj.hasKey(m_tileConfigKey)) {
      m_tileConfig = ZJsonObject(obj.value(m_tileConfigKey));
    }

    if (obj.hasKey(m_roiListKey)) {
      ZJsonArray jsonArray(obj.value(m_roiListKey));
      for (size_t i = 0; i < jsonArray.size(); ++i) {
        addRoiName(ZJsonParser::stringValue(jsonArray.getData(), i));
      }
    }
    if (obj.hasKey(m_roiNameKey)) {
      setRoiName(ZJsonParser::stringValue(obj[m_roiNameKey]));
    }
    if (obj.hasKey(m_synapseNameKey)) {
      setSynapseName(ZJsonParser::stringValue(obj[m_synapseNameKey]));
    }

    if (obj.hasKey(m_todoListNameKey)) {
      setTodoListName(ZJsonParser::stringValue(obj[m_todoListNameKey]));
    }

    if (obj.hasKey(m_defaultSettingKey)) {
      useDefaultDataSetting(ZJsonParser::booleanValue(obj[m_defaultSettingKey]));
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

    if (obj.hasKey(m_sourceConfigKey)) {
      m_sourceConfig.set(obj.value(m_sourceConfigKey));
    }
  }
}

std::string ZDvidTarget::getUrl() const
{
  return m_node.getUrl();
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
  } else if (ZDvidData::IsNullName(m_bodyLabelName)) {
    return "";
  }

  return m_bodyLabelName;
}

std::string ZDvidTarget::getBodyLabelName(int zoom) const
{
  std::string name;
  if (zoom == 0) {
    return getBodyLabelName();
  } else if (zoom > 0) {
    name = getBodyLabelName() + "_" + ZString::num2str(zoom);
  }

  return name;
}

bool ZDvidTarget::usingMulitresBodylabel() const
{
  if (m_usingMultresBodyLabel) {
    return getMaxLabelZoom() > 3;
  }

  return false;
}

bool ZDvidTarget::hasBodyLabel() const
{
  return !getBodyLabelName().empty();
}

bool ZDvidTarget::hasLabelBlock() const
{
  return !getLabelBlockName().empty();
}

std::string ZDvidTarget::getLabelBlockName() const
{ 
  if (m_labelBlockName.empty()) {
    return ZDvidData::GetName(ZDvidData::ROLE_LABEL_BLOCK);
  } else if (ZDvidData::IsNullName(m_labelBlockName)) {
    return "";
  }

  return m_labelBlockName;
}

std::string ZDvidTarget::GetMultiscaleDataName(
    const std::string &dataName, int zoom)
{
  std::string name = dataName;

  if (!name.empty() && zoom > 0) {
    name = name + "_" + ZString::num2str(zoom);
  }

  return name;
}

std::string ZDvidTarget::getLabelBlockName(int zoom) const
{
  return GetMultiscaleDataName(getLabelBlockName(), zoom);
  /*
  std::string name = getLabelBlockName();
  if (!name.empty() && zoom > 0) {
    name = name + "_" + ZString::num2str(zoom);
  }

  return name;
  */
}

std::string ZDvidTarget::getGrayScaleName(int zoom) const
{
  return GetMultiscaleDataName(getGrayScaleName(), zoom);
}

std::string ZDvidTarget::getValidGrayScaleName(int zoom) const
{
  if (zoom < 0 || zoom > getMaxGrayscaleZoom()) {
    return "";
  }

  return getGrayScaleName(zoom);
}

std::string ZDvidTarget::getValidLabelBlockName(int zoom) const
{
  if (zoom < 0 || zoom > getMaxLabelZoom()) {
    return "";
  }

  return getLabelBlockName(zoom);
}

void ZDvidTarget::setLabelBlockName(const std::string &name)
{
  m_labelBlockName = name;
}

std::string ZDvidTarget::getMultiscale2dName() const
{
  return m_multiscale2dName;
}

bool ZDvidTarget::hasTileData() const
{
  return !getMultiscale2dName().empty();
}

bool ZDvidTarget::isTileLowQuality() const
{
  return isLowQualityTile(getMultiscale2dName());
}

std::string ZDvidTarget::getBodyInfoName() const
{
  return ZDvidData::GetName(ZDvidData::ROLE_BODY_INFO,
                            ZDvidData::ROLE_BODY_LABEL,
                            getBodyLabelName());
}

bool ZDvidTarget::isLowQualityTile(const std::string &name) const
{
  bool lowQuality = true;

  if (!name.empty()) {
    lowQuality = false;
    if (m_tileConfig.hasKey(name.c_str())) {
      ZJsonObject obj(m_tileConfig.value(name.c_str()));
      if (obj.hasKey("low_quality")) {
        lowQuality = ZJsonParser::booleanValue(obj["low_quality"]);
      }
    }
  }

  return lowQuality;
}

bool ZDvidTarget::hasGrayScaleData() const
{
  if (getGrayScaleName() == "*") {
    return false;
  }

  return true;
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

void ZDvidTarget::setNullBodyLabelName()
{
  setBodyLabelName("*");
}

void ZDvidTarget::setNullLabelBlockName()
{
  setLabelBlockName("*");
}

void ZDvidTarget::setMultiscale2dName(const std::string &name)
{
  m_multiscale2dName = name;
}

void ZDvidTarget::setDefaultMultiscale2dName()
{
  m_multiscale2dName = ZDvidData::GetName(ZDvidData::ROLE_MULTISCALE_2D);
}

void ZDvidTarget::configTile(const std::string &name, bool lowQuality)
{
  if (!name.empty()) {
    if (lowQuality) {
      ZJsonObject obj;
      if (m_tileConfig.hasKey(name.c_str())) {
        obj = ZJsonObject(m_tileConfig.value(name.c_str()));
      } else {
        m_tileConfig.setEntry(name.c_str(), obj);
      }
      obj.setEntry("low_quality", true);
    } else {
      if (m_tileConfig.hasKey(name.c_str())) {
        ZJsonObject obj(m_tileConfig.value(name.c_str()));
        if (obj.hasKey("low_quality")) {
          obj.setEntry("low_quality", false);
        }
      }
    }
  }
}

std::string ZDvidTarget::getRoiName(size_t index) const
{
  return m_roiList[index];
}

std::string ZDvidTarget::getRoiName() const
{
  return m_roiName;
}

void ZDvidTarget::setRoiName(const std::string &name)
{
  m_roiName = name;
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

void ZDvidTarget::setTodoListName(const std::string &name)
{
  m_todoListName = name;
}

std::string ZDvidTarget::getTodoListName() const
{
  if (!m_todoListName.empty()) {
    return m_todoListName;
  }

  return ZDvidData::GetName(ZDvidData::ROLE_TODO_LIST,
                            ZDvidData::ROLE_BODY_LABEL,
                            getBodyLabelName());
}

std::string ZDvidTarget::getBodyAnnotationName() const
{
  return ZDvidData::GetName(ZDvidData::ROLE_BODY_ANNOTATION,
                            ZDvidData::ROLE_BODY_LABEL, getBodyLabelName());
}

void ZDvidTarget::setSynapseName(const std::string &name)
{
  m_synapseName = name;
}


std::string ZDvidTarget::getSplitLabelName() const
{
  return ZDvidData::GetName(
        ZDvidData::ROLE_SPLIT_LABEL, ZDvidData::ROLE_BODY_LABEL,
        getBodyLabelName());
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

void ZDvidTarget::setSourceConfig(const ZJsonObject &config)
{
  m_sourceConfig = config;
}

void ZDvidTarget::setSource(const char *key, const ZDvidNode &node)
{
  if (node == m_node || !node.isValid()) {
    m_sourceConfig.removeKey(key);
  } else {
    m_sourceConfig.setEntry(key, node.toJsonObject().getData());
  }
}

void ZDvidTarget::setGrayScaleSource(const ZDvidNode &node)
{
  setSource(m_grayScaleNameKey, node);
}

ZDvidNode ZDvidTarget::getSource(const char *key) const
{
  if (m_sourceConfig.hasKey(key)) {
    ZDvidNode node;
    ZJsonObject obj(m_sourceConfig.value(key));
    node.loadJsonObject(obj);
    if (node.isValid()) {
      return node;
    }
  }

  return m_node;
}

ZDvidNode ZDvidTarget::getGrayScaleSource() const
{
  return getSource(m_grayScaleNameKey);
}

ZDvidNode ZDvidTarget::getTileSource() const
{
  return getSource(m_multiscale2dNameKey);
}

void ZDvidTarget::setTileSource(const ZDvidNode &node)
{
  setSource(m_multiscale2dNameKey, node);
}


void ZDvidTarget::prepareGrayScale()
{
  if (m_sourceConfig.hasKey(m_grayScaleNameKey)) {
    ZJsonObject nodeJson(m_sourceConfig.value(m_grayScaleNameKey));
    m_node.loadJsonObject(nodeJson);
  }
}

void ZDvidTarget::prepareTile()
{
  if (m_sourceConfig.hasKey(m_multiscale2dNameKey)) {
    ZJsonObject nodeJson(m_sourceConfig.value(m_multiscale2dNameKey));
    m_node.loadJsonObject(nodeJson);
  }
}

bool ZDvidTarget::isDvidTarget(const std::string &source)
{
  return ZString(source).startsWith("http:");
}
